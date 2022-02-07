#pragma once

namespace Workshop
{
	class PlacementMode :
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
		public RE::BSTEventSink<RE::Workshop::ItemPlacedEvent>,
		public RE::BSTEventSink<RE::Workshop::WorkshopModeEvent>
	{
	public:
		class Hooks
		{
		public:
			template<std::uint64_t id, std::ptrdiff_t offset>
			class hkCanNavigate
			{
			public:
				static void Install()
				{
					REL::Relocation<std::uintptr_t> target{ REL::ID(id), offset };

					auto& trampoline = F4SE::GetTrampoline();
					func = trampoline.write_call<5>(target.address(), thunk);
				}

			private:
				static bool thunk()
				{
					return PlacementMode::IsActive() ? false : func();
				}

				static inline REL::Relocation<decltype(&thunk)> func;
			};

			template<std::uint64_t id, std::ptrdiff_t offset>
			class hkShouldShowTagForSearch
			{
			public:
				static void Install()
				{
					REL::Relocation<std::uintptr_t> target{ REL::ID(id), offset };

					auto& trampoline = F4SE::GetTrampoline();
					func = trampoline.write_call<5>(target.address(), thunk);
				}

			private:
				static std::uint64_t thunk(RE::WorkshopMenu* a_this)
				{
					return PlacementMode::IsActive() ? 0 : func(a_this);
				}

				static inline REL::Relocation<decltype(&thunk)> func;
			};

			static void Install()
			{
				// Prevent stored frames from stacking
				REL::Relocation<std::uintptr_t> targetCImpl{ RE::ExtraStartingWorldOrCell::VTABLE[0] };
				targetCImpl.write_vfunc(0x01, hkCompareImpl);

				// Redirect Cancel input, block other buttons
				REL::Relocation<std::uintptr_t> targetEvent{ RE::WorkshopMenu::VTABLE[1] };
				ogHandleEvent = targetEvent.write_vfunc(0x08, hkHandleEvent);

				// Disable selection in UI
				hkCanNavigate<119865, 0x37A>::Install();
				hkCanNavigate<119865, 0x3EA>::Install();
				hkCanNavigate<119865, 0x4C5>::Install();
				hkCanNavigate<119865, 0x768>::Install();
				hkCanNavigate<985073, 0x0C>::Install();
				hkCanNavigate<1130413, 0x0C>::Install();

				// Don't allow tagging for search when you're not using resources
				hkShouldShowTagForSearch<119865, 0xEBB>::Install();
				hkShouldShowTagForSearch<1089189, 0x574>::Install();

				// Enable ExtraStartingWorldOrCell as a stacking condition
				REL::Relocation<std::uintptr_t> targetUIQ{ REL::ID(179412) };
				stl::asm_replace(targetUIQ.address(), 0x1C7, reinterpret_cast<std::uintptr_t>(hkUIQualifier));

				// Prevent Workshops marked as deleted as being valid
				REL::Relocation<std::uintptr_t> targetRWA{ REL::ID(1322808) };
				stl::asm_replace(targetRWA.address(), 0x066, reinterpret_cast<std::uintptr_t>(hkIsReferenceWithinBuildableArea));

				// Patch a nullptr exception
				REL::Relocation<std::uintptr_t> targetCGR{ REL::ID(44523) };
				stl::asm_replace(targetCGR.address(), 0x03F, reinterpret_cast<std::uintptr_t>(hkCanGoRight));
			}

		private:
			static bool hkCompareImpl(RE::ExtraStartingWorldOrCell* a_this, const RE::ExtraStartingWorldOrCell& a_compare)
			{
				if (!a_this)
				{
					return true;
				}

				if (a_this->type != RE::ExtraStartingWorldOrCell::TYPE ||
					a_compare.type != RE::ExtraStartingWorldOrCell::TYPE)
				{
					return true;
				}

				if (a_this->startingWorldOrCell->GetFormType() == RE::ENUM_FORM_ID::kCELL ||
					a_this->startingWorldOrCell->GetFormType() == RE::ENUM_FORM_ID::kWRLD ||
					a_compare.startingWorldOrCell->GetFormType() == RE::ENUM_FORM_ID::kCELL ||
					a_compare.startingWorldOrCell->GetFormType() == RE::ENUM_FORM_ID::kWRLD)
				{
					return false;
				}

				return a_this->startingWorldOrCell != a_compare.startingWorldOrCell;
			}

			static void hkHandleEvent(RE::BSInputEventUser* a_this, const RE::ButtonEvent* a_event)
			{
				if (a_event && PlacementMode::IsActive())
				{
					if (a_event->QUserEvent() == "XButton" ||
						a_event->QUserEvent() == "YButton" ||
						a_event->QUserEvent() == "LShoulder" ||
						a_event->QUserEvent() == "RShoulder" ||
						a_event->QUserEvent() == "LTrigger" ||
						a_event->QUserEvent() == "RTrigger" ||
						a_event->QUserEvent() == "Sprint" ||
						a_event->QUserEvent() == "Jump")
					{
						return;
					}

					if (a_event->QUserEvent() == "Cancel")
					{
						auto _event =
							stl::unrestricted_cast<RE::ButtonEvent*>(a_event);
						_event->strUserEvent = "CloseMenu";
						return ogHandleEvent(a_this, _event);
					}
				}

				ogHandleEvent(a_this, a_event);
			}

			static bool hkUIQualifier(const RE::BSExtraData* a_extra)
			{
				if (!a_extra)
				{
					return false;
				}

				switch (a_extra->type.get())
				{
					// case RE::EXTRA_DATA_TYPE::kPersistentCell:
					// case RE::EXTRA_DATA_TYPE::kKeywords:
					// case RE::EXTRA_DATA_TYPE::kStartingPosition:
					// case RE::EXTRA_DATA_TYPE::kReferenceHandle:
					// case RE::EXTRA_DATA_TYPE::kOwnership:
					// case RE::EXTRA_DATA_TYPE::kGlobal:
					// case RE::EXTRA_DATA_TYPE::kRank:
					case RE::EXTRA_DATA_TYPE::kHealth:
					// case RE::EXTRA_DATA_TYPE::kTimeLeft:
					case RE::EXTRA_DATA_TYPE::kCharge:
					// case RE::EXTRA_DATA_TYPE::kLevelItem:
					// case RE::EXTRA_DATA_TYPE::kScale:
					case RE::EXTRA_DATA_TYPE::kObjectInstance:
					case RE::EXTRA_DATA_TYPE::kCannotWear:
					case RE::EXTRA_DATA_TYPE::kPoison:
					case RE::EXTRA_DATA_TYPE::kBoundArmor:
					case RE::EXTRA_DATA_TYPE::kStartingWorldOrCell:
					// case RE::EXTRA_DATA_TYPE::kFavorite:
					// case RE::EXTRA_DATA_TYPE::kAliasInstanceArray:
					// case RE::EXTRA_DATA_TYPE::kPromotedRef:
					// case RE::EXTRA_DATA_TYPE::kOutfitItem:
					// case RE::EXTRA_DATA_TYPE::kFromAlias:
					// case RE::EXTRA_DATA_TYPE::kShouldWear:
					case RE::EXTRA_DATA_TYPE::kTextDisplayData:
					case RE::EXTRA_DATA_TYPE::kEnchantment:
					// case RE::EXTRA_DATA_TYPE::kUniqueID:
					// case RE::EXTRA_DATA_TYPE::kFlags:
					case RE::EXTRA_DATA_TYPE::kInstanceData:
					case RE::EXTRA_DATA_TYPE::kModRank:
						return true;

					default:
						return false;
				}
			}

			static bool hkIsReferenceWithinBuildableArea(const RE::TESObjectREFR& a_workshop, const RE::TESObjectREFR& a_refr)
			{
				if ((a_workshop.formFlags & (1 << 5)) != 0)
				{
					return false;
				}

				auto rfParentCell = a_refr.parentCell;
				if (rfParentCell)
				{
					auto wsParentCell = a_workshop.parentCell;

					RE::TESWorldSpace* wsWorldSpace{ nullptr };
					if (wsParentCell && wsParentCell->IsExterior())
					{
						wsWorldSpace = rfParentCell->worldSpace;
					}

					if (rfParentCell->IsInterior() || !rfParentCell->worldSpace)
					{
						if (wsWorldSpace || rfParentCell != wsParentCell)
						{
							return false;
						}
					}
					else if (wsWorldSpace != rfParentCell->worldSpace)
					{
						return false;
					}
				}

				return RE::Workshop::IsLocationWithinBuildableArea(a_workshop, a_refr.data.location);
			}

			static bool hkCanGoRight()
			{
				std::uint32_t column{ 0 };
				auto selectedWorkshopMenuNode = RE::Workshop::GetSelectedWorkshopMenuNode(*RE::Workshop::CurrentRow, column);
				return selectedWorkshopMenuNode &&
					   selectedWorkshopMenuNode->parent &&
					   column < selectedWorkshopMenuNode->parent->children.size() - 1;
			}

			static inline REL::Relocation<decltype(&hkHandleEvent)> ogHandleEvent;
		};

		[[nodiscard]] static PlacementMode* GetSingleton()
		{
			static PlacementMode singleton;
			return std::addressof(singleton);
		}

		static void ApplyPerk()
		{
			auto PAPerk = Forms::PAFramePerk_DO->GetForm<RE::BGSPerk>();
			if (!PAPerk)
			{
				return;
			}

			if (auto Player = RE::TESForm::GetFormByID(0x00000007)->As<RE::TESNPC>())
			{
				Player->AddPerk(PAPerk, 1);
			}
		}

		static void Start()
		{
			if (auto workbench = Forms::PAFrameWorkshop_DO->GetForm<RE::TESObjectCONT>())
			{
				auto singleton = GetSingleton();
				if (singleton->m_workshop = CreateWorkbench(workbench))
				{
					if (auto UI = RE::UI::GetSingleton())
					{
						if (UI->GetMenuOpen("PipboyMenu"))
						{
							RE::UIMessageQueue::GetSingleton()->AddMessage(
								"PipboyMenu",
								RE::UI_MESSAGE_TYPE::kHide);
						}
					}

					singleton->m_isActive = true;
					RE::Workshop::RegisterForWorkshopModeEvent(singleton);
					RE::Workshop::StartWorkshop(singleton->m_workshop.get().get());
				}
			}
		}

		static void SetFrameReference(RE::TESObjectREFR* a_refr)
		{
			GetSingleton()->m_frameRefr.reset(a_refr);
		}

		static void SetTokenReference(RE::TESObjectREFR* a_refr)
		{
			GetSingleton()->m_tokenRefr.reset(a_refr);
		}

		static bool IsActive()
		{
			return GetSingleton()->m_isActive;
		}

		static bool CreateToken(RE::TESObjectREFR* a_refr)
		{
			if (auto KYWD = RE::TESForm::GetFormByID<RE::BGSKeyword>(0x0003430B))
			{
				if (a_refr && a_refr->HasKeyword(KYWD))
				{
					if (auto TOKN = Forms::PAFrameToken_DO->GetForm<RE::TESObjectARMO>())
					{
						auto DATA =
							RE::BSTSmartPointer(new RE::ExtraDataList());
						DATA->SetStartingWorldOrCell(a_refr);

						auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
						if (!PlayerCharacter)
						{
							return false;
						}

						if (auto DPM = RE::BGSDynamicPersistenceManager::GetSingleton())
						{
							if (!DPM->PromoteReference(a_refr, PlayerCharacter))
							{
								logger::warn("Failed to promote PA Frame reference.");
								return false;
							}
						}

						PlayerCharacter->AddObjectToContainer(
							TOKN,
							DATA,
							1,
							nullptr,
							RE::ITEM_REMOVE_REASON::kNone);
						a_refr->Disable();

						return true;
					}
				}
			}

			return false;
		}

		static bool HandleToken(RE::TESObjectREFR* a_refr)
		{
			if (auto TOKN = Forms::PAFrameToken_DO->GetForm<RE::TESObjectARMO>())
			{
				if (a_refr && a_refr->data.objectReference && a_refr->data.objectReference->formID == TOKN->formID)
				{
					if (a_refr->extraList &&
						a_refr->extraList->HasType<RE::ExtraStartingWorldOrCell>())
					{
						if (auto DATA = a_refr->extraList->GetByType<RE::ExtraStartingWorldOrCell>())
						{
							if (DATA->startingWorldOrCell)
							{
								if (auto REFR = DATA->startingWorldOrCell->As<RE::TESObjectREFR>())
								{
									SetTokenReference(a_refr);
									SetFrameReference(REFR);
									a_refr->Disable();

									Start();
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override
		{
			if (a_event.menuName == RE::WorkshopMenu::MENU_NAME)
			{
				if (a_event.opening)
				{
					if (auto menu = RE::UI::GetSingleton()->GetMenu<RE::WorkshopMenu>())
					{
						menu->workshopMenuBase->itemName->SetMember("visible", false);
						menu->workshopMenuBase->selectionBracket->SetMember("visible", false);
						menu->workshopMenuBase->itemCounts->SetMember("visible", false);
						menu->workshopMenuBase->newRecipeIcon->SetMember("visible", false);
						menu->workshopMenuBase->rowBrackets->SetMember("visible", false);
						menu->workshopMenuBase->displayPath->SetMember("visible", false);
						menu->workshopMenuBase->descriptionBase->SetMember("visible", false);
						menu->workshopMenuBase->iconBackground->SetMember("visible", false);

						menu->workshopMenuBase->Invoke("HideRequirements");
						menu->workshopMenuBase->Invoke("HideIconCard");
						menu->workshopMenuBase->Invoke("HidePerkPanels");

						(*RE::Workshop::CurrentRow)++;

						menu->CheckAndSetItemForPlacement();
						menu->UpdateButtonText();

						if (auto& handle = *RE::Workshop::PlacementItem)
						{
							if (m_frameRefr && m_frameRefr->inventoryList)
							{
								if (!handle.get()->inventoryList)
								{
									handle.get()->CreateInventoryList(nullptr);
								}

								RE::BSAutoReadLock{ m_frameRefr->inventoryList->rwLock };
								RE::BSAutoWriteLock{ handle.get()->inventoryList->rwLock };
								for (auto& iter : m_frameRefr->inventoryList->data)
								{
									handle.get()->inventoryList->data.emplace_back(iter);
								}
							}
						}
					}
				}
				else
				{
					a_source->UnregisterSink(this);
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::Workshop::ItemPlacedEvent& a_event, RE::BSTEventSource<RE::Workshop::ItemPlacedEvent>*) override
		{
			if (a_event.workshop == m_workshop.get())
			{
				if (auto DPM = RE::BGSDynamicPersistenceManager::GetSingleton())
				{
					DPM->DemoteReference(m_frameRefr.get(), RE::PlayerCharacter::GetSingleton());
				}

				m_frameRefr->SetDelete(true);
				m_frameRefr->SetWantsDelete(true);
				m_frameRefr->Disable();

				m_tokenRefr->SetDelete(true);
				m_tokenRefr->SetWantsDelete(true);
				m_tokenRefr->Disable();

				RE::Workshop::RequestExitWorkshop(false);
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::Workshop::WorkshopModeEvent& a_event, RE::BSTEventSource<RE::Workshop::WorkshopModeEvent>*) override
		{
			if (a_event.start)
			{
				RE::UI::GetSingleton()->RegisterSink<RE::MenuOpenCloseEvent>(this);
				RE::Workshop::RegisterForItemPlaced(this);
			}
			else
			{
				RE::Workshop::UnregisterForItemPlaced(this);
				RE::Workshop::UnregisterForWorkshopModeEvent(this);

				m_workshop.get()->SetDelete(true);
				m_workshop.get()->SetWantsDelete(true);
				m_workshop.get()->Disable();
				m_workshop.reset();

				if (m_tokenRefr && !m_tokenRefr->GetDelete())
				{
					RE::PlayerCharacter::GetSingleton()->AddObjectToContainer(
						m_tokenRefr->data.objectReference,
						m_tokenRefr->extraList,
						1,
						nullptr,
						RE::ITEM_REMOVE_REASON::kNone);

					m_tokenRefr->SetDelete(true);
					m_tokenRefr->SetWantsDelete(true);
					m_tokenRefr->Disable();
				}

				SetTokenReference(nullptr);
				SetFrameReference(nullptr);

				m_isActive = false;
			}

			return RE::BSEventNotifyControl::kContinue;
		}

	private:
		[[nodiscard]] static RE::ObjectRefHandle CreateWorkbench(RE::TESBoundObject* a_workbench)
		{
			if (auto PlayerCharacter = RE::PlayerCharacter::GetSingleton())
			{
				auto data = RE::NEW_REFR_DATA();
				data.location = PlayerCharacter->data.location;
				data.direction = PlayerCharacter->data.angle;
				data.interior = PlayerCharacter->parentCell;
				data.world = data.interior ? data.interior->worldSpace : nullptr;
				data.object = a_workbench;

				if (auto TESDataHandler = RE::TESDataHandler::GetSingleton())
				{
					return TESDataHandler->CreateReferenceAtLocation(data);
				}
			}

			return RE::ObjectRefHandle();
		}

		RE::ObjectRefHandle m_workshop;
		RE::BSTSmartPointer<RE::TESObjectREFR, RE::BSTSmartPointerGamebryoRefCount> m_frameRefr;
		RE::BSTSmartPointer<RE::TESObjectREFR, RE::BSTSmartPointerGamebryoRefCount> m_tokenRefr;
		bool m_isActive{ false };
	};
}
