#pragma once

namespace Patches
{
	namespace LoadEditorIDs
	{
		namespace
		{
			class Patch
			{
			public:
				static const char* GetFormEditorID(RE::TESForm* a_this)
				{
					auto iter = rmap.find(a_this);
					if (iter != rmap.end())
					{
						return iter->second.c_str();
					}

					return func_get(a_this);
				}

				static bool SetFormEditorID(RE::TESForm* a_this, const char* a_editor)
				{
					if (a_this->formID < 0xFF000000 && !std::string_view(a_editor).empty())
					{
						AddToGameMap(a_this, a_editor);
					}

					return func_set(a_this, a_editor);
				}

				static inline REL::Relocation<decltype(&RE::TESForm::GetFormEditorID)> func_get;
				static inline REL::Relocation<decltype(&RE::TESForm::SetFormEditorID)> func_set;

			private:
				static void AddToGameMap(RE::TESForm* a_this, const char* a_editorID)
				{
					const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
					const RE::BSAutoWriteLock locker{ lock.get() };
					if (map)
					{
						if (*Settings::Patches::EnableEDIDConflictCheck)
						{
							auto iter = map->find(a_editorID);
							if (iter != map->end())
							{
								logger::warn(FMT_STRING("EditorID Conflict: {:08X} and {:08X} are both {:s}"sv), iter->second->GetFormID(), a_this->GetFormID(), a_editorID);
							}
						}

						map->emplace(a_editorID, a_this);
						rmap.emplace(a_this, a_editorID);
					}
				}

				// reverse lookup map
				static inline RE::BSTHashMap<RE::TESForm*, RE::BSFixedString> rmap;
			};

			template<class Form>
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> vtbl{ Form::VTABLE[0] };
				Patch::func_get = vtbl.write_vfunc(0x3A, Patch::GetFormEditorID);
				Patch::func_set = vtbl.write_vfunc(0x3B, Patch::SetFormEditorID);
			}
		}

		void InstallHooks()
		{
			// InstallHook<RE::BGSKeyword>();
			// InstallHook<RE::BGSLocationRefType>();
			// InstallHook<RE::BGSAction>();
			InstallHook<RE::BGSTransform>();
			InstallHook<RE::BGSComponent>();
			InstallHook<RE::BGSTextureSet>();
			// InstallHook<RE::BGSMenuIcon>();
			// InstallHook<RE::TESGlobal>();
			InstallHook<RE::BGSDamageType>();
			InstallHook<RE::TESClass>();
			InstallHook<RE::TESFaction>();
			// InstallHook<RE::BGSHeadPart>();
			InstallHook<RE::TESEyes>();
			// InstallHook<RE::TESRace>();
			// InstallHook<RE::TESSound>();
			InstallHook<RE::BGSAcousticSpace>();
			InstallHook<RE::EffectSetting>();
			InstallHook<RE::Script>();
			InstallHook<RE::TESLandTexture>();
			InstallHook<RE::EnchantmentItem>();
			InstallHook<RE::SpellItem>();
			InstallHook<RE::ScrollItem>();
			InstallHook<RE::TESObjectACTI>();
			InstallHook<RE::BGSTalkingActivator>();
			InstallHook<RE::TESObjectARMO>();
			InstallHook<RE::TESObjectBOOK>();
			InstallHook<RE::TESObjectCONT>();
			InstallHook<RE::TESObjectDOOR>();
			InstallHook<RE::IngredientItem>();
			InstallHook<RE::TESObjectLIGH>();
			InstallHook<RE::TESObjectMISC>();
			InstallHook<RE::TESObjectSTAT>();
			// InstallHook<RE::BGSStaticCollection>();
			// InstallHook<RE::BGSMovableStatic>();
			InstallHook<RE::TESGrass>();
			InstallHook<RE::TESObjectTREE>();
			InstallHook<RE::TESFlora>();
			InstallHook<RE::TESFurniture>();
			InstallHook<RE::TESObjectWEAP>();
			InstallHook<RE::TESAmmo>();
			InstallHook<RE::TESNPC>();
			InstallHook<RE::TESLevCharacter>();
			InstallHook<RE::TESKey>();
			InstallHook<RE::AlchemyItem>();
			InstallHook<RE::BGSIdleMarker>();
			InstallHook<RE::BGSNote>();
			InstallHook<RE::BGSProjectile>();
			InstallHook<RE::BGSHazard>();
			InstallHook<RE::BGSBendableSpline>();
			InstallHook<RE::TESSoulGem>();
			InstallHook<RE::BGSTerminal>();
			InstallHook<RE::TESLevItem>();
			InstallHook<RE::TESWeather>();
			InstallHook<RE::TESClimate>();
			InstallHook<RE::BGSShaderParticleGeometryData>();
			InstallHook<RE::BGSReferenceEffect>();
			InstallHook<RE::TESRegion>();
			// InstallHook<RE::NavMeshInfoMap>();
			// InstallHook<RE::TESObjectCELL>();
			// InstallHook<RE::TESObjectREFR>();
			InstallHook<RE::Explosion>();
			InstallHook<RE::Projectile>();
			InstallHook<RE::Actor>();
			InstallHook<RE::PlayerCharacter>();
			InstallHook<RE::MissileProjectile>();
			InstallHook<RE::ArrowProjectile>();
			InstallHook<RE::GrenadeProjectile>();
			InstallHook<RE::BeamProjectile>();
			InstallHook<RE::FlameProjectile>();
			InstallHook<RE::ConeProjectile>();
			InstallHook<RE::BarrierProjectile>();
			InstallHook<RE::Hazard>();
			// InstallHook<RE::TESWorldSpace>();
			// InstallHook<RE::TESObjectLAND>();
			// InstallHook<RE::NavMesh>();
			// InstallHook<RE::TESTopic>();
			InstallHook<RE::TESTopicInfo>();
			// InstallHook<RE::TESQuest>();
			// InstallHook<RE::TESIdleForm>();
			InstallHook<RE::TESPackage>();
			InstallHook<RE::AlarmPackage>();
			InstallHook<RE::DialoguePackage>();
			InstallHook<RE::FleePackage>();
			InstallHook<RE::SpectatorPackage>();
			InstallHook<RE::TrespassPackage>();
			InstallHook<RE::TESCombatStyle>();
			InstallHook<RE::TESLoadScreen>();
			InstallHook<RE::TESLevSpell>();
			// InstallHook<RE::TESObjectANIO>();
			InstallHook<RE::TESWaterForm>();
			InstallHook<RE::TESEffectShader>();
			InstallHook<RE::BGSExplosion>();
			InstallHook<RE::BGSDebris>();
			InstallHook<RE::TESImageSpace>();
			// InstallHook<RE::TESImageSpaceModifier>();
			InstallHook<RE::BGSListForm>();
			InstallHook<RE::BGSPerk>();
			InstallHook<RE::BGSBodyPartData>();
			InstallHook<RE::BGSAddonNode>();
			// InstallHook<RE::ActorValueInfo>();
			InstallHook<RE::BGSCameraShot>();
			InstallHook<RE::BGSCameraPath>();
			// InstallHook<RE::BGSVoiceType>();
			InstallHook<RE::BGSMaterialType>();
			InstallHook<RE::BGSImpactData>();
			InstallHook<RE::BGSImpactDataSet>();
			InstallHook<RE::TESObjectARMA>();
			InstallHook<RE::BGSEncounterZone>();
			InstallHook<RE::BGSLocation>();
			InstallHook<RE::BGSMessage>();
			// InstallHook<RE::BGSDefaultObjectManager>();
			// InstallHook<RE::BGSDefaultObject>();
			InstallHook<RE::BGSLightingTemplate>();
			// InstallHook<RE::BGSMusicType>();
			InstallHook<RE::BGSFootstep>();
			InstallHook<RE::BGSFootstepSet>();
			// InstallHook<RE::BGSStoryManagerBranchNode>();
			// InstallHook<RE::BGSStoryManagerQuestNode>();
			// InstallHook<RE::BGSStoryManagerEventNode>();
			InstallHook<RE::BGSDialogueBranch>();
			InstallHook<RE::BGSMusicTrackFormWrapper>();
			InstallHook<RE::TESWordOfPower>();
			InstallHook<RE::TESShout>();
			InstallHook<RE::BGSEquipSlot>();
			InstallHook<RE::BGSRelationship>();
			InstallHook<RE::BGSScene>();
			InstallHook<RE::BGSAssociationType>();
			InstallHook<RE::BGSOutfit>();
			InstallHook<RE::BGSArtObject>();
			InstallHook<RE::BGSMaterialObject>();
			InstallHook<RE::BGSMovementType>();
			// InstallHook<RE::BGSSoundDescriptorForm>();
			InstallHook<RE::BGSDualCastData>();
			InstallHook<RE::BGSSoundCategory>();
			InstallHook<RE::BGSSoundOutput>();
			InstallHook<RE::BGSCollisionLayer>();
			InstallHook<RE::BGSColorForm>();
			InstallHook<RE::BGSReverbParameters>();
			// InstallHook<RE::BGSPackIn>();
			InstallHook<RE::BGSAimModel>();
			InstallHook<RE::BGSConstructibleObject>();
			InstallHook<RE::BGSMod::Attachment::Mod>();
			InstallHook<RE::BGSMaterialSwap>();
			InstallHook<RE::BGSZoomData>();
			InstallHook<RE::BGSInstanceNamingRules>();
			InstallHook<RE::BGSSoundKeywordMapping>();
			InstallHook<RE::BGSAudioEffectChain>();
			// InstallHook<RE::BGSAttractionRule>();
			InstallHook<RE::BGSSoundCategorySnapshot>();
			InstallHook<RE::BGSSoundTagSet>();
			InstallHook<RE::BGSLensFlare>();
			InstallHook<RE::BGSGodRays>();

			logger::debug("Installed Patch: LoadEditorIDs"sv);
		}
	}
}
