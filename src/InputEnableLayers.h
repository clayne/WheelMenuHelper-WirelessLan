#pragma once

namespace InputEnableLayers {
	bool g_inputEnableLayerEnabled;
	uint32_t g_inputEnableLayerIndex;

	struct InputEnableLayerState {
		uint32_t index;
		uint32_t state;
	};

	class BSInputEnableManager {
	public:
		enum {
			kUserEvent_Movement = 1 << 0,
			kUserEvent_Looking = 1 << 1,
			kUserEvent_Activate = 1 << 2,
			kUserEvent_Menu = 1 << 3,
			kUserEvent_Console = 1 << 4,
			kUserEvent_POVChange = 1 << 5,
			kUserEvent_Fighting = 1 << 6,
			kUserEvent_Sneaking = 1 << 7,
			kUserEvent_MainFourMenu = 1 << 8,
			kUserEvent_WheelZoom = 1 << 9,
			kUserEvent_Jumping = 1 << 10
		};

		enum {
			kOtherEvent_JournalTabs = 1 << 0,
			kOtherEvent_Activation = 1 << 1,
			kOtherEvent_FastTravel = 1 << 2,
			kOtherEvent_POVChange = 1 << 3,
			kOtherEvent_VATS = 1 << 4,
			kOtherEvent_FAVORITES = 1 << 5,
			kOtherEvent_PipboyLight = 1 << 6,
			kOtherEvent_ZKey = 1 << 7,
			kOtherEvent_Running = 1 << 8
		};

		uint64_t unk00[0x118 >> 3];
		uint64_t currentInputEnableMask;			// 118
		uint64_t unk120;
		RE::BSSpinLock inputEnableArrLock;			// 128
		RE::BSTArray<uint64_t>	inputEnableMaskArr;	// 130
		RE::BSTArray<InputEnableLayerState*> layerStateArr;	// 148
		RE::BSTArray<void*> unk160;					// 160
	};

	REL::Relocation<BSInputEnableManager**> InputEnableManager{ REL::ID(781703) };

	bool EnableUserEvent(BSInputEnableManager* inputEnableManager, uint32_t layerIdx, uint32_t flag, bool enable, uint32_t arg4) {
		using func_t = decltype(&EnableUserEvent);
		REL::Relocation<func_t> func{ REL::ID(1432984) };

		return func(inputEnableManager, layerIdx, flag, enable, arg4);
	}

	bool EnableOtherEvent(BSInputEnableManager* inputEnableManager, uint32_t layerIdx, uint32_t flag, bool enable, uint32_t arg4) {
		using func_t = decltype(&EnableOtherEvent);
		REL::Relocation<func_t> func{ REL::ID(1419268) };

		return func(inputEnableManager, layerIdx, flag, enable, arg4);
	}

	bool SetInputEnableLayer(uint32_t& layerIdx, uint32_t userEventFlag, uint32_t otherEventFlag) {
		BSInputEnableManager** g_inputEnableManager = InputEnableManager.get();
		if (!g_inputEnableManager || !*g_inputEnableManager)
			return false;

		(*g_inputEnableManager)->inputEnableArrLock.lock();

		layerIdx = 0xFFFFFFFF;
		for (InputEnableLayerState* layerState : (*g_inputEnableManager)->layerStateArr) {
			if (layerState->state == 1) {
				layerState->state = 2;
				layerIdx = layerState->index;
				break;
			}
		}

		(*g_inputEnableManager)->inputEnableArrLock.unlock();

		if (layerIdx == 0xFFFFFFFF)
			return false;

		if (!EnableUserEvent(*g_inputEnableManager, layerIdx, userEventFlag, false, 3))
			return false;

		if (!EnableOtherEvent(*g_inputEnableManager, layerIdx, otherEventFlag, false, 3))
			return false;

		return true;
	}

	void SetInputEnableLayer() {
		if (!g_inputEnableLayerEnabled) {
			g_inputEnableLayerEnabled = true;
			uint32_t userEventFlag = BSInputEnableManager::kUserEvent_Looking | BSInputEnableManager::kUserEvent_MainFourMenu;
			uint32_t otherEventFlag = BSInputEnableManager::kOtherEvent_JournalTabs | BSInputEnableManager::kOtherEvent_Activation 
								| BSInputEnableManager::kOtherEvent_VATS | BSInputEnableManager::kOtherEvent_FAVORITES | BSInputEnableManager::kOtherEvent_POVChange;
			SetInputEnableLayer(g_inputEnableLayerIndex, userEventFlag, otherEventFlag);
		}
	}

	bool ResetInputEnableLayer(uint32_t layerIdx) {
		BSInputEnableManager** g_inputEnableManager = InputEnableManager.get();
		if (!g_inputEnableManager || !*g_inputEnableManager)
			return false;

		uint64_t flag = 0xFFFFFFFFFFFFFFFF;
		EnableUserEvent(*g_inputEnableManager, layerIdx, flag & 0xFFFFFFFF, true, 3);
		EnableOtherEvent(*g_inputEnableManager, layerIdx, flag >> 32, true, 3);

		(*g_inputEnableManager)->inputEnableArrLock.lock();
		(*g_inputEnableManager)->layerStateArr[layerIdx]->state = 1;
		(*g_inputEnableManager)->inputEnableArrLock.unlock();

		return true;
	}

	void ResetInputEnableLayer() {
		if (g_inputEnableLayerEnabled) {
			g_inputEnableLayerEnabled = false;
			ResetInputEnableLayer(g_inputEnableLayerIndex);
		}
	}
}
