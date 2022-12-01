#include "InputEnableLayers.h"
#include "Utils.h"

const F4SE::stl::zstring menuName = "F4WheelMenu"sv;

bool OpenMenu(RE::UI* g_ui) {
	if (!g_ui || g_ui->GetMenuOpen(menuName) || Utils::IsInMenuMode(g_ui))
		return false;

	RE::UIMessageQueue* g_uiMessageQueue = RE::UIMessageQueue::GetSingleton();
	if (!g_uiMessageQueue)
		return false;

	InputEnableLayers::SetInputEnableLayer();

	g_uiMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kShow);
	return true;
}

bool CloseMenu(RE::UI* g_ui) {
	if (!g_ui || !g_ui->GetMenuOpen(menuName))
		return false;

	RE::UIMessageQueue* g_uiMessageQueue = RE::UIMessageQueue::GetSingleton();
	if (!g_uiMessageQueue)
		return false;

	InputEnableLayers::ResetInputEnableLayer();

	g_uiMessageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide);
	return true;
}

bool IsMenuOpen(RE::UI* g_ui) {
	if (!g_ui || !Utils::IsMenuRegistered(g_ui, menuName))
		return false;

	return g_ui->GetMenuOpen(menuName);
}

bool OpenMenu_Method(std::monostate) {
	RE::UI* g_ui = RE::UI::GetSingleton();
	if (!g_ui)
		return false;

	return OpenMenu(g_ui);
}

bool CloseMenu_Method(std::monostate) {
	RE::UI* g_ui = RE::UI::GetSingleton();
	if (!g_ui)
		return false;

	return CloseMenu(g_ui);
}

bool IsMenuOpen_Method(std::monostate) {
	RE::UI* g_ui = RE::UI::GetSingleton();
	if (!g_ui)
		return false;

	return IsMenuOpen(g_ui);
}

void OnF4SEMessage(F4SE::MessagingInterface::Message* msg) {
	switch (msg->type) {
	case F4SE::MessagingInterface::kNewGame:
	case F4SE::MessagingInterface::kPreLoadGame:
		InputEnableLayers::ResetInputEnableLayer();
		break;
	}
}
bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* a_vm) {
	a_vm->BindNativeMethod("WheelMenuHelper"sv, "OpenMenu"sv, OpenMenu_Method);
	a_vm->BindNativeMethod("WheelMenuHelper"sv, "CloseMenu"sv, CloseMenu_Method);
	a_vm->BindNativeMethod("WheelMenuHelper"sv, "IsMenuOpen"sv, IsMenuOpen_Method);
	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface * a_f4se, F4SE::PluginInfo * a_info) {
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor"sv);
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical(FMT_STRING("unsupported runtime v{}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface * a_f4se) {
	F4SE::Init(a_f4se);

	const F4SE::MessagingInterface* message = F4SE::GetMessagingInterface();
	if (message)
		message->RegisterListener(OnF4SEMessage);

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	if (papyrus)
		papyrus->Register(RegisterPapyrusFunctions);

	return true;
}
