#pragma once

namespace Events::Handlers
{
	class ItemCraftedHandler :
		public RE::BSTEventSink<RE::ItemCrafted::Event>,
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		virtual ~ItemCraftedHandler()
		{
			m_refr.reset();
		}

		[[nodiscard]] static ItemCraftedHandler* GetSingleton()
		{
			static ItemCraftedHandler singleton;
			return std::addressof(singleton);
		}

		static void Register()
		{
			RE::ItemCrafted::RegisterSink(GetSingleton());
			if (auto UI = RE::UI::GetSingleton())
			{
				UI->RegisterSink<RE::MenuOpenCloseEvent>(GetSingleton());
			}
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::ItemCrafted::Event& a_event, RE::BSTEventSource<RE::ItemCrafted::Event>*) override
		{
			if (auto StoryEventManager = RE::BGSStoryEventManager::GetSingleton())
			{
				RE::BGSLocation* location{ nullptr };
				if (auto parentCell = m_refr ? m_refr->GetParentCell() : nullptr)
				{
					location = parentCell->GetLocation();
				}

				auto item = a_event.recipe ? a_event.recipe->GetCreatedItem() : nullptr;
				StoryEventManager->AddEvent(RE::BGSCraftItemEvent{ m_refr.get(), location, item });
			}

			return RE::BSEventNotifyControl::kContinue;
		}

		virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (a_event.menuName == "ExamineMenu" || a_event.menuName == "CookingMenu" || a_event.menuName == "RobotModMenu")
			{
				if (a_event.opening)
				{
					if (auto UI = RE::UI::GetSingleton())
					{
						auto menu = static_cast<RE::WorkbenchMenuBase*>(UI->GetMenu(a_event.menuName).get());
						if (menu && menu->workbenchRef)
						{
							m_refr.reset(menu->workbenchRef.get());
						}
					}
				}
				else
				{
					m_refr.reset();
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}

	private:
		RE::BSTSmartPointer<RE::TESObjectREFR, RE::BSTSmartPointerGamebryoRefCount> m_refr;
	};
}
