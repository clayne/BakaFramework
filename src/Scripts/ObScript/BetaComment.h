#pragma once

namespace ObScript
{
	class BetaComment
	{
	public:
		static void Install()
		{
			const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
			const auto it = std::find_if(
				functions.begin(),
				functions.end(),
				[&](auto&& a_elem)
				{
					return _stricmp(a_elem.functionName, "BetaComment") == 0;
				});

			if (it != functions.end())
			{
				static std::array params{
					RE::SCRIPT_PARAMETER{"String", RE::SCRIPT_PARAM_TYPE::kChar, false},
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				DEBUG("Registered BetaComment."sv);
			}
			else
			{
				DEBUG("Failed to register BetaComment."sv);
			}
		}

	private:
		static bool Execute(
			const RE::SCRIPT_PARAMETER* a_parameters,
			const char* a_compiledParams,
			RE::TESObjectREFR* a_refObject,
			RE::TESObjectREFR* a_container,
			RE::Script* a_script,
			RE::ScriptLocals* a_scriptLocals,
			float&,
			std::uint32_t& a_offset)
		{
			std::array<char, 0x200> rawComment{ '\0' };
			auto result = RE::Script::ParseParameters(
				a_parameters,
				a_compiledParams,
				a_offset,
				a_refObject,
				a_container,
				a_script,
				a_scriptLocals,
				rawComment.data());

			if (!result)
			{
				return true;
			}

			if (rawComment[0] == '\0')
			{
				return true;
			}

			m_refr = a_refObject;
			if (!m_refr)
			{
				RE::ConsoleLog::GetSingleton()->PrintLine("Using Player for BetaComment.");
				m_refr = RE::PlayerCharacter::GetSingleton();
			}

			/*
				%D (%R)							| Current Time
				%s								| File Name
				%D (%R)							| File Time
				%s								| Machine Name
				%08X							| FormID
				%s								| Form Name
				%s								| Cell Name (Int: %s, Ext: %s (%d,%d))
				%08X							| Cell FormID
				%.0f							| Location X
				%.0f							| Location Y
				%.0f							| Location Z
				%.0f							| Angle X
				%.0f							| Angle Y
				%.0f							| Angle Z
				%.0f							| Camera Pos X
				%.0f							| Camera Pos Y
				%.0f							| Camera Pos Z
				%.0f							| Camera Orientation X
				%.0f							| Camera Orientation Y
				%.0f							| Camera Orientation Z
				"%s"							| Comment
			*/

			std::stringstream line;

			PrintCurrentTime(line);
			PrintFileInfo(line);
			PrintMachineName(line);
			PrintFormInfo(line);
			PrintCellInfo(line);
			PrintRefrPositionInfo(line);
			PrintCameraPositionInfo(line);

			DEBUG(""sv);

			// Print Comment
			line << "\"" << rawComment.data() << "\"";
			line << "\r\n";

			_file.open(*Config::Patches::sBetaCommentFileName, std::ofstream::out | std::ofstream::app);
			_file << line.str();
			_file.close();

			m_refr = nullptr;
			return true;
		}

		static bool PrintCurrentTime(std::stringstream& a_buf)
		{
			auto currentTime_t = std::time(nullptr);
			auto currentTime = fmt::format("{:%m/%d/%y (%H:%M)}"sv, fmt::localtime(currentTime_t));
			DEBUG("CurrentTime: {:s}"sv, currentTime);

			a_buf << currentTime << _delim;
			return true;
		}

		static bool PrintFileInfo(std::stringstream& a_buf)
		{
			auto file = m_refr->GetFile(0);
			if (file)
			{
				auto fileName = file->GetFilename();
				auto fileTime = fmt::format("{:%m/%d/%y (%H:%M)}"sv, fmt::localtime(GetFileTime(file)));
				DEBUG("File Name: {:s}"sv, fileName);
				DEBUG("File Time: {:s}"sv, fileTime);

				a_buf << fileName << _delim << fileTime << _delim;
			}
			else
			{
				DEBUG("Warning: No File."sv);
				a_buf << _delim << _delim;
			}

			return true;
		}

		static bool PrintMachineName(std::stringstream& a_buf)
		{
			const auto machineName = []() -> std::string
			{
				char buffer[16];
				std::uint32_t bufferSize = sizeof(buffer) / sizeof(char);

				auto result = F4SE::WinAPI::GetComputerName(buffer, &bufferSize);
				return (result) ? buffer : "UNKNOWN"s;
			}();

			DEBUG("Machine Name: {:s}"sv, machineName);

			a_buf << machineName << _delim;
			return true;
		}

		static bool PrintFormInfo(std::stringstream& a_buf)
		{
			auto formID = fmt::format("{:08X}"sv, m_refr->formID);
			auto formName = m_refr->GetFormEditorID();
			DEBUG("FormID: {:s}"sv, formID);
			DEBUG("Form Name: {:s}"sv, formName);

			a_buf << formID << _delim << formName << _delim;
			return true;
		}

		static bool PrintCellInfo(std::stringstream& a_buf)
		{
			std::stringstream temp;

			auto cell = m_refr->GetParentCell();
			if (cell)
			{
				auto cellID = fmt::format("{:08X}"sv, cell->formID);

				if (cell->cellFlags.all(RE::TESObjectCELL::Flag::kInterior))
				{
					auto cellName = cell->GetFormEditorID();
					DEBUG("Cell (Interior): {:s}"sv, cellName);
					DEBUG("Cell FormID (Interior): {:s}"sv, cellID);
					temp << cellName << _delim << cellID << _delim;
				}
				else
				{
					if ((cell->formFlags >> 13) & 1)
					{
						WARN("BetaComment::PrintCellInfo: Unknown Edge Case."sv);
						temp << _delim << _delim;
					}
					else
					{
						auto cellX = cell->GetDataX();
						auto cellY = cell->GetDataY();
						auto cellName = fmt::format(
							"{:s} ({:d},{:d})"sv,
							cell->worldSpace->GetFormEditorID(),
							cellX,
							cellY);
						DEBUG("Cell (Exterior): {:s}"sv, cellName);
						DEBUG("Cell FormID (Exterior): {:s}"sv, cellID);
						temp << cellName << _delim << cellID << _delim;
					}
				}
			}
			else
			{
				DEBUG("Cell Info: No Cell."sv);
				temp << _delim << _delim;
			}

			a_buf << temp.str();
			return true;
		}

		static bool PrintRefrPositionInfo(std::stringstream& a_buf)
		{
			auto locationX = fmt::format("{:.0f}"sv, m_refr->data.location.x);
			auto locationY = fmt::format("{:.0f}"sv, m_refr->data.location.y);
			auto locationZ = fmt::format("{:.0f}"sv, m_refr->data.location.z);
			DEBUG("Refr Location: {:s}, {:s}, {:s}"sv, locationX, locationY, locationZ);

			auto angleX = fmt::format("{:.0f}"sv, m_refr->data.angle.x);
			auto angleY = fmt::format("{:.0f}"sv, m_refr->data.angle.y);
			auto angleZ = fmt::format("{:.0f}"sv, m_refr->data.angle.z);
			DEBUG("Refr Angle: {:s}, {:s}, {:s}"sv, angleX, angleY, angleZ);

			a_buf
				<< locationX << _delim
				<< locationY << _delim
				<< locationZ << _delim
				<< angleX << _delim
				<< angleY << _delim
				<< angleZ << _delim;
			return true;
		}

		static bool PrintCameraPositionInfo(std::stringstream& a_buf)
		{
			auto rootCamera = RE::Main::WorldRootCamera();
			auto cameraPositionX = fmt::format("{:.0f}"sv, rootCamera->world.translate.x);
			auto cameraPositionY = fmt::format("{:.0f}"sv, rootCamera->world.translate.y);
			auto cameraPositionZ = fmt::format("{:.0f}"sv, rootCamera->world.translate.z);
			DEBUG("Camera Position: {:s}, {:s}, {:s}"sv, cameraPositionX, cameraPositionY, cameraPositionZ);

			float fCameraAngleX{ 0.0f }, fCameraAngleY{ 0.0f }, fCameraAngleZ{ 0.0f };
			rootCamera->parent->world.rotate.ToEulerAnglesXYZ(fCameraAngleX, fCameraAngleY, fCameraAngleZ);
			auto cameraAngleX = fmt::format("{:.0f}"sv, fCameraAngleX);
			auto cameraAngleY = fmt::format("{:.0f}"sv, fCameraAngleY);
			auto cameraAngleZ = fmt::format("{:.0f}"sv, fCameraAngleZ);
			DEBUG("Camera Angle: {:s}, {:s}, {:s}"sv, cameraAngleX, cameraAngleY, cameraAngleZ);

			a_buf
				<< cameraPositionX << _delim
				<< cameraPositionY << _delim
				<< cameraPositionZ << _delim
				<< cameraAngleX << _delim
				<< cameraAngleY << _delim
				<< cameraAngleZ << _delim;
			return true;
		}

		[[nodiscard]] static const std::string& HelpString()
		{
			static auto help = []()
			{
				std::string buf;
				buf += "Add comment to "sv;
				buf += *Config::Patches::sBetaCommentFileName;
				buf += ". [bc \"This is clipping.\"]"sv;
				return buf;
			}();
			return help;
		}

		[[nodiscard]] static std::time_t GetFileTime(RE::TESFile* a_file)
		{
			union
			{
				struct
				{
					std::uint32_t LowPart;
					std::uint32_t HighPart;
				} s;

				std::uint64_t QuadPart;
			} ull;

			ull.s.LowPart = a_file->fileInfo.modifyTime.dwLowDateTime;
			ull.s.HighPart = a_file->fileInfo.modifyTime.dwHighDateTime;

			return static_cast<std::time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);
		}

		static constexpr auto LONG_NAME = "BetaComment"sv;
		static constexpr auto SHORT_NAME = "bc"sv;

		inline static RE::TESObjectREFR* m_refr{ nullptr };
		inline static char _delim{ '\t' };

		inline static std::ofstream _file;
	};
}
