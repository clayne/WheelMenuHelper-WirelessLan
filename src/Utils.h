#pragma once

namespace Utils {
	bool IsInMenuMode(RE::UI* g_ui) {
		if (!g_ui)
			return false;

		return g_ui->menuMode != 0;
	}

	bool IsMenuRegistered(RE::UI* g_ui, const RE::BSFixedString& menuName) {
		if (!g_ui)
			return false;

		RE::BSAutoReadLock l{ RE::UI::GetMenuMapRWLock() };
		const auto it = g_ui->menuMap.find(menuName);
		return it != g_ui->menuMap.end();
	}
}
