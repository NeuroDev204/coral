/*
FxSound
Copyright (C) 2025  FxSound LLC

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <JuceHeader.h>
#include "FxSystemTrayView.h"
#include "../Utils/SysInfo/SysInfo.h"

#if defined(_WIN32)

// {A8E96325-5269-443C-A0D8-0D02562FE553}
const GUID FxSystemTrayView::trayIconGuid_ =
{ 0xa8e96325, 0x5269, 0x443c, { 0xa0, 0xd8, 0xd, 0x2, 0x56, 0x2f, 0xe5, 0x53 } };


FxSystemTrayView::FxSystemTrayView()
{
    FxModel::getModel().addListener(this);

    custom_notification_ = true;

    addToDesktop(0);

    HWND hWnd = (HWND)getWindowHandle();

    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    componentWndProc_ = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)wndProc);

    taskbar_created_message_ = RegisterWindowMessage(TEXT("TaskbarCreated"));

    addIcon();
}

FxSystemTrayView::~FxSystemTrayView()
{
    FxModel::getModel().removeListener(this);

    HWND hWnd = (HWND)getWindowHandle();

    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)componentWndProc_);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = trayIconGuid_;
    Shell_NotifyIcon(NIM_DELETE, &nid);

    removeFromDesktop();
}

void FxSystemTrayView::modelChanged(FxModel::Event model_event)
{
    if (!FxController::getInstance().isNotificationsHidden() && model_event == FxModel::Event::Notification)
    {
        showNotification();
    }
}

void FxSystemTrayView::setStatus(bool power, bool processing)
{
    HINSTANCE hInst = GetModuleHandle(NULL);

    String param = power ? TRANS(L"on") : TRANS(L"off");

    wchar_t tool_tip[1024];
    swprintf_s(tool_tip, String(TRANS("Coral is %s.")).toWideCharPointer(), param.toWideCharPointer());
	wcscat_s(tool_tip, 1024, L"\n\n");
    wcscat_s(tool_tip, 1024, String(TRANS("Output: ")).toWideCharPointer());
    auto& model = FxModel::getModel();
	String output_device_name = model.getSelectedOutput().deviceFriendlyName.c_str();
	wcscat_s(tool_tip, 1024, output_device_name.toWideCharPointer());

    NOTIFYICONDATA nid = { sizeof(nid) };

    nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = trayIconGuid_;
    if (power)
    {
        if (processing)
        {
            if (FxTheme::getThemeMode() == FxThemeMode::Dark)
            {
                nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_RED");
            }
            else
            {
                nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_BLUE");
            }
        }
        else
        {
            nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_WHITE");
        }
    }
    else
    {
        nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_GRAY");
    }

    if (nid.hIcon == NULL)
    {
        return;
    }

    lstrcpy(nid.szTip, tool_tip);

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

Point<int> FxSystemTrayView::getSystemTrayWindowPosition(int width, int height)
{
    Point<int> pos = { 0, 0 };

    NOTIFYICONIDENTIFIER icon_id = {};
    RECT rect;

    HWND hWnd = (HWND)getWindowHandle();

    icon_id.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    icon_id.hWnd = hWnd;
    icon_id.guidItem = trayIconGuid_;

    if (FAILED(Shell_NotifyIconGetRect(&icon_id, &rect)))
    {
        return pos;
    }

    auto display = Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display != nullptr)
    {
        juce::Rectangle<int> prect{ rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
        auto lrect = Desktop::getInstance().getDisplays().physicalToLogical(prect);

        auto area = display->userArea;

        if (lrect.getX() < area.getCentreX())
        {
            pos.x = area.getX() + 10;
        }
        else
        {
            pos.x = area.getRight() - width - 10;
        }


        if (lrect.getY() < area.getCentreY())
        {
            pos.y = area.getY() + 10;
        }
        else
        {
            pos.y = area.getBottom() - height - 10;
        }
    }

    return pos;
}

void FxSystemTrayView::addIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid) };

    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND hWnd = (HWND)getWindowHandle();

    if (FxModel::getModel().getPowerState())
    {
        if (FxController::getInstance().isAudioProcessing())
        {
            if (FxTheme::getThemeMode() == FxThemeMode::Dark)
            {
                nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_RED");
            }
            else
            {
                nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_BLUE");
            }
        }
        else
        {
            nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_WHITE");
        }
    }
    else
    {
        nid.hIcon = LoadIcon(hInst, L"IDI_LOGO_GRAY");
    }

    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = trayIconGuid_;
    nid.uCallbackMessage = WMAPP_FXTRAYICON;
    nid.hWnd = hWnd;
    lstrcpy(nid.szTip, L"Coral");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);

    setVisible(true);
}

void FxSystemTrayView::showContextMenu()
{
    PopupMenu context_menu;

    PopupMenu preset_menu;
    PopupMenu theme_menu;

    FxController::getInstance().checkDeviceChanges();

    auto id = PRESET_MENU_ID_START;
    auto count = FxModel::getModel().getPresetCount();
    auto preset_type = FxModel::PresetType::AppPreset;
    for (auto i = 0; i < count; i++)
    {
        auto preset = FxModel::getModel().getPreset(i);
        PopupMenu::Item menu_item(preset.modified ? preset.name + L" *" : preset.name);
        menu_item.setID(id);
        if (id - PRESET_MENU_ID_START == FxModel::getModel().getSelectedPreset())
        {
            menu_item.setTicked(true);
        }

        if (preset_type != preset.type)
        {
            preset_menu.addSeparator();
            preset_type = preset.type;
        }

        auto handler = [preset = id]() {
            FxController::getInstance().setPreset(preset - PRESET_MENU_ID_START);
        };
        menu_item.setAction(handler);
        preset_menu.addItem(menu_item);
        id++;
    }

    context_menu.addSubMenu(TRANS("Presets"), preset_menu);
    context_menu.addSeparator();

    context_menu.addItem(MENU_ID_OPEN, TRANS("Open Coral"),
        !FxController::getInstance().isWindowVisible(), false);
    context_menu.addItem(MENU_ID_POWER,
        FxModel::getModel().getPowerState() ? TRANS("Power off") : TRANS("Power on"));

    {
        auto& model = FxModel::getModel();
        String output_device_name = model.getSelectedOutput().deviceFriendlyName.c_str();
        PopupMenu output_menu;
        addOutputDeviceMenu(&output_menu);
        context_menu.addSubMenu(TRANS("Output device") + ": " + output_device_name, output_menu);
    }

    theme_menu.addItem(1001, TRANS("Dark"), true,
        FxTheme::getThemeMode() == FxThemeMode::Dark, []() { FxTheme::setThemeMode(FxThemeMode::Dark); });
    theme_menu.addItem(1002, TRANS("Light"), true,
        FxTheme::getThemeMode() == FxThemeMode::Light, []() { FxTheme::setThemeMode(FxThemeMode::Light); });
    theme_menu.addItem(1003, TRANS("System default"), true,
        FxTheme::getThemeMode() == FxThemeMode::SystemDefault,
        []() { FxTheme::setThemeMode(FxThemeMode::SystemDefault); });
    context_menu.addSubMenu(TRANS("Theme"), theme_menu);

    context_menu.addItem(MENU_ID_SETTINGS, TRANS("Settings"));
    context_menu.addItem(MENU_ID_DONATE, TRANS("Donate"));
    context_menu.addSeparator();
    context_menu.addItem(MENU_ID_EXIT, TRANS("Exit"));

    PopupMenu::Options opt;
    opt.setTargetComponent(this);
    context_menu.showMenu(opt);
}

void FxSystemTrayView::addOutputDeviceMenu(PopupMenu* context_menu)
{
    // Linux: simplified - just show the default output
    auto selected = FxModel::getModel().getSelectedOutput();

    PopupMenu::Item default_item(selected.deviceFriendlyName.c_str());
    default_item.setID(OUTPUT_MENU_ID_START);
    default_item.setTicked(true);
    context_menu->addItem(default_item);
}

void FxSystemTrayView::showNotification()
{
    // Linux: use JUCE notification API
    notification_.setMessage("Coral is enhancing your audio", {"", ""}, true);
    notification_.showMessage();
}

String FxSystemTrayView::getTruncatedText(const String& text, int max_length)
{
    if (text.length() <= max_length) return text;
    return text.substring(0, max_length - 3) + "...";
}

LRESULT CALLBACK FxSystemTrayView::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WMAPP_FXTRAYICON)
    {
        if (LOWORD(lParam) == WM_RBUTTONUP)
        {
            FxSystemTrayView* view =
                reinterpret_cast<FxSystemTrayView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (view != nullptr)
            {
                view->showContextMenu();
            }
        }
        else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
        {
            FxSystemTrayView* view =
                reinterpret_cast<FxSystemTrayView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (view != nullptr)
            {
                FxController::getInstance().showMainWindow();
            }
        }
    }
    else if (message == WM_COMMAND)
    {
        FxSystemTrayView* view =
            reinterpret_cast<FxSystemTrayView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (view != nullptr)
        {
            switch (LOWORD(wParam))
            {
                case MENU_ID_OPEN:
                    FxController::getInstance().showMainWindow();
                    break;
                case MENU_ID_POWER:
                    FxController::getInstance().powerOn(!FxModel::getModel().getPowerState());
                    break;
                case MENU_ID_SETTINGS:
                    FxController::getInstance().showSettingsDialog();
                    break;
                case MENU_ID_DONATE:
                    FxController::getInstance().showDonateDialog();
                    break;
                case MENU_ID_EXIT:
                    FxController::getInstance().exit();
                    break;
            }
        }
    }
    else if (message == taskbar_created_message_)
    {
        FxSystemTrayView* view =
            reinterpret_cast<FxSystemTrayView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (view != nullptr)
        {
            view->addIcon();
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

#else // Linux: AppIndicator (GNOME) with GtkStatusIcon fallback

#include <dlfcn.h>
#include <functional>
#include <iostream>
#include <gtk/gtk.h>

namespace
{
    enum
    {
        kAppIndicatorCategoryApplicationStatus = 0
    };

    enum
    {
        kAppIndicatorStatusPassive = 0,
        kAppIndicatorStatusActive = 1
    };

    struct AppIndicator;

    using AppIndicatorNewFn = AppIndicator* (*)(const gchar*, const gchar*, int);
    using AppIndicatorSetStatusFn = void (*)(AppIndicator*, int);
    using AppIndicatorSetMenuFn = void (*)(AppIndicator*, GtkMenu*);
    using AppIndicatorSetTitleFn = void (*)(AppIndicator*, const gchar*);
    using AppIndicatorSetIconFn = void (*)(AppIndicator*, const gchar*);
    using AppIndicatorSetIconThemePathFn = void (*)(AppIndicator*, const gchar*);
    using AppIndicatorSetSecondaryFn = void (*)(AppIndicator*, GtkWidget*);

    void* g_indicator_lib = nullptr;
    AppIndicator* g_indicator = nullptr;
    GtkStatusIcon* g_tray_icon = nullptr;
    GtkMenu* g_context_menu = nullptr;
    GtkWidget* g_menu_open = nullptr;
    GtkWidget* g_menu_power = nullptr;
    GtkWidget* g_menu_settings = nullptr;
    GtkWidget* g_menu_donate = nullptr;
    GtkWidget* g_menu_exit = nullptr;
    AppIndicatorSetTitleFn g_set_title = nullptr;
    AppIndicatorSetIconFn g_set_icon = nullptr;
    AppIndicatorSetIconThemePathFn g_set_icon_theme_path = nullptr;

    File coralIconFile(int px)
    {
        const File home = File::getSpecialLocation(File::userHomeDirectory);
        File local = home.getChildFile(".local")
            .getChildFile("share")
            .getChildFile("icons")
            .getChildFile("hicolor")
            .getChildFile(String(px) + "x" + String(px))
            .getChildFile("apps")
            .getChildFile("coral.png");
        if (local.existsAsFile())
            return local;

        File pixmaps = home.getChildFile(".local")
            .getChildFile("share")
            .getChildFile("pixmaps")
            .getChildFile("coral.png");
        if (pixmaps.existsAsFile())
            return pixmaps;

        return File::getSpecialLocation(File::currentExecutableFile).getSiblingFile("coral.png");
    }

    String coralIconThemeDir()
    {
        const File home = File::getSpecialLocation(File::userHomeDirectory);
        return home.getChildFile(".local").getChildFile("share").getChildFile("icons").getFullPathName();
    }

    void dispatchToJuce(std::function<void()> fn)
    {
        MessageManager::callAsync(std::move(fn));
    }

    void onMenuOpen(GtkMenuItem*, gpointer)
    {
        dispatchToJuce([] {
            FxController::getInstance().showMainWindow();
        });
    }

    void onMenuPower(GtkMenuItem*, gpointer)
    {
        dispatchToJuce([] {
            FxController::getInstance().setPowerState(!FxModel::getModel().getPowerState());
        });
    }

    void onMenuSettings(GtkMenuItem*, gpointer)
    {
        dispatchToJuce([] {
            FxController::getInstance().showMainWindow();
            FxSettingsDialog settings_dialog;
            settings_dialog.runModalLoop();
            FxController::getInstance().refreshOutputList();
        });
    }

    void onMenuDonate(GtkMenuItem*, gpointer)
    {
        dispatchToJuce([] {
            URL("https://www.paypal.com/donate/?hosted_button_id=JVNQGYXCQ2GPG").launchInDefaultBrowser();
        });
    }

    void onMenuExit(GtkMenuItem*, gpointer)
    {
        dispatchToJuce([] {
            FxController::getInstance().exit();
        });
    }

    void onTrayActivate(GtkStatusIcon*, gpointer)
    {
        dispatchToJuce([] {
            auto& controller = FxController::getInstance();
            if (controller.isMainWindowVisible())
                controller.hideMainWindow();
            else
                controller.showMainWindow();
        });
    }

    void onTrayPopup(GtkStatusIcon*, guint, guint activate_time, gpointer)
    {
        if (g_context_menu == nullptr)
            return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_menu_popup(g_context_menu, nullptr, nullptr,
                       gtk_status_icon_position_menu, g_tray_icon,
                       0, activate_time);
#pragma GCC diagnostic pop
    }

    GtkMenu* createTrayMenu()
    {
        GtkMenu* menu = GTK_MENU(gtk_menu_new());

        g_menu_open = gtk_menu_item_new_with_label("Open Coral");
        g_menu_power = gtk_menu_item_new_with_label(
            FxModel::getModel().getPowerState() ? "Power off" : "Power on");
        g_menu_settings = gtk_menu_item_new_with_label("Settings");
        g_menu_donate = gtk_menu_item_new_with_label("Donate");
        g_menu_exit = gtk_menu_item_new_with_label("Exit");

        gtk_menu_shell_append(GTK_MENU_SHELL(menu), g_menu_open);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), g_menu_power);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), g_menu_settings);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), g_menu_donate);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), g_menu_exit);

        g_signal_connect(g_menu_open, "activate", G_CALLBACK(onMenuOpen), nullptr);
        g_signal_connect(g_menu_power, "activate", G_CALLBACK(onMenuPower), nullptr);
        g_signal_connect(g_menu_settings, "activate", G_CALLBACK(onMenuSettings), nullptr);
        g_signal_connect(g_menu_donate, "activate", G_CALLBACK(onMenuDonate), nullptr);
        g_signal_connect(g_menu_exit, "activate", G_CALLBACK(onMenuExit), nullptr);

        gtk_widget_show_all(GTK_WIDGET(menu));
        return menu;
    }

    bool tryCreateAppIndicator()
    {
        g_indicator_lib = dlopen("libayatana-appindicator3.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (g_indicator_lib == nullptr)
            g_indicator_lib = dlopen("libappindicator3.so.1", RTLD_LAZY | RTLD_LOCAL);

        if (g_indicator_lib == nullptr)
        {
            std::cerr << "Coral: AppIndicator library not found\n";
            return false;
        }

        auto new_fn = reinterpret_cast<AppIndicatorNewFn>(dlsym(g_indicator_lib, "app_indicator_new"));
        auto set_status = reinterpret_cast<AppIndicatorSetStatusFn>(dlsym(g_indicator_lib, "app_indicator_set_status"));
        auto set_menu = reinterpret_cast<AppIndicatorSetMenuFn>(dlsym(g_indicator_lib, "app_indicator_set_menu"));
        g_set_title = reinterpret_cast<AppIndicatorSetTitleFn>(dlsym(g_indicator_lib, "app_indicator_set_title"));
        g_set_icon = reinterpret_cast<AppIndicatorSetIconFn>(dlsym(g_indicator_lib, "app_indicator_set_icon"));
        g_set_icon_theme_path = reinterpret_cast<AppIndicatorSetIconThemePathFn>(
            dlsym(g_indicator_lib, "app_indicator_set_icon_theme_path"));
        auto set_secondary = reinterpret_cast<AppIndicatorSetSecondaryFn>(
            dlsym(g_indicator_lib, "app_indicator_set_secondary_activate_target"));

        if (new_fn == nullptr || set_status == nullptr || set_menu == nullptr)
        {
            std::cerr << "Coral: AppIndicator symbols missing\n";
            dlclose(g_indicator_lib);
            g_indicator_lib = nullptr;
            return false;
        }

        const char* icon_name = "coral";
        GtkIconTheme* theme = gtk_icon_theme_get_default();
        if (theme == nullptr || !gtk_icon_theme_has_icon(theme, icon_name))
        {
            icon_name = "audio-volume-high";
        }

        g_indicator = new_fn("coral", icon_name, kAppIndicatorCategoryApplicationStatus);
        if (g_indicator == nullptr)
        {
            std::cerr << "Coral: app_indicator_new failed\n";
            return false;
        }

        const String theme_dir = coralIconThemeDir();
        if (g_set_icon_theme_path != nullptr)
            g_set_icon_theme_path(g_indicator, theme_dir.toRawUTF8());
        if (g_set_icon != nullptr)
            g_set_icon(g_indicator, "coral");

        set_menu(g_indicator, g_context_menu);
        set_status(g_indicator, kAppIndicatorStatusActive);

        if (g_set_title != nullptr)
            g_set_title(g_indicator, "Coral");

        if (set_secondary != nullptr && g_menu_open != nullptr)
            set_secondary(g_indicator, g_menu_open);

        std::cerr << "Coral: AppIndicator tray ready\n";
        return true;
    }

    void createStatusIconFallback()
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        g_tray_icon = gtk_status_icon_new();
        if (g_tray_icon == nullptr)
        {
            std::cerr << "Coral: GtkStatusIcon fallback failed\n";
            return;
        }

        GdkPixbuf* pixbuf = nullptr;
        const File icon_file = coralIconFile(22);
        if (icon_file.existsAsFile())
            pixbuf = gdk_pixbuf_new_from_file(icon_file.getFullPathName().toRawUTF8(), nullptr);

        GtkIconTheme* theme = gtk_icon_theme_get_default();
        if (pixbuf == nullptr && theme != nullptr)
        {
            if (gtk_icon_theme_has_icon(theme, "coral"))
                pixbuf = gtk_icon_theme_load_icon(theme, "coral", 22, static_cast<GtkIconLookupFlags>(0), nullptr);
            if (pixbuf == nullptr && gtk_icon_theme_has_icon(theme, "audio-volume-high"))
                pixbuf = gtk_icon_theme_load_icon(theme, "audio-volume-high", 22, static_cast<GtkIconLookupFlags>(0), nullptr);
        }

        if (pixbuf != nullptr)
        {
            gtk_status_icon_set_from_pixbuf(g_tray_icon, pixbuf);
            g_object_unref(pixbuf);
        }

        gtk_status_icon_set_title(g_tray_icon, "Coral");
        gtk_status_icon_set_tooltip_text(g_tray_icon, "Coral");
        g_signal_connect(g_tray_icon, "activate", G_CALLBACK(onTrayActivate), nullptr);
        g_signal_connect(g_tray_icon, "popup-menu", G_CALLBACK(onTrayPopup), nullptr);
        gtk_status_icon_set_visible(g_tray_icon, TRUE);
#pragma GCC diagnostic pop

        std::cerr << "Coral: GtkStatusIcon tray fallback ready\n";
    }
}

FxSystemTrayView::FxSystemTrayView()
{
    FxModel::getModel().addListener(this);
    custom_notification_ = true;

    if (!gtk_init_check(nullptr, nullptr))
    {
        std::cerr << "Coral: GTK init failed - tray icon disabled\n";
        return;
    }

    g_context_menu = createTrayMenu();

    if (!tryCreateAppIndicator())
        createStatusIconFallback();

    // AppIndicator / GtkStatusIcon live on the GLib loop. JUCE's X11 loop
    // does not pump it, so clicks would never arrive without this timer.
    startTimer(50);
}

FxSystemTrayView::~FxSystemTrayView()
{
    stopTimer();
    FxModel::getModel().removeListener(this);

    if (g_tray_icon != nullptr)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_status_icon_set_visible(g_tray_icon, FALSE);
#pragma GCC diagnostic pop
        g_object_unref(g_tray_icon);
        g_tray_icon = nullptr;
    }

    if (g_indicator != nullptr)
    {
        g_object_unref(g_indicator);
        g_indicator = nullptr;
    }

    if (g_context_menu != nullptr)
    {
        gtk_widget_destroy(GTK_WIDGET(g_context_menu));
        g_context_menu = nullptr;
        g_menu_open = nullptr;
        g_menu_power = nullptr;
        g_menu_settings = nullptr;
        g_menu_donate = nullptr;
        g_menu_exit = nullptr;
    }

    g_set_title = nullptr;
    g_set_icon = nullptr;
}

void FxSystemTrayView::timerCallback()
{
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
}

void FxSystemTrayView::modelChanged(FxModel::Event model_event)
{
    if (!FxController::getInstance().isNotificationsHidden() && model_event == FxModel::Event::Notification)
        showNotification();
}

void FxSystemTrayView::setStatus(bool power, bool processing)
{
    ignoreUnused(processing);

    const char* tooltip = power ? "Coral (On)" : "Coral (Off)";

    if (g_set_title != nullptr && g_indicator != nullptr)
        g_set_title(g_indicator, tooltip);

    if (g_tray_icon != nullptr)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_status_icon_set_tooltip_text(g_tray_icon, tooltip);
#pragma GCC diagnostic pop
    }

    if (g_menu_power != nullptr)
    {
        gtk_menu_item_set_label(GTK_MENU_ITEM(g_menu_power),
            power ? "Power off" : "Power on");
    }
}

Point<int> FxSystemTrayView::getSystemTrayWindowPosition(int width, int height)
{
    Point<int> pos = { 0, 0 };

    auto display = Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display != nullptr)
    {
        auto area = display->userArea;
        pos.x = area.getRight() - width - 10;
        pos.y = area.getY() + 10;
    }

    return pos;
}

void FxSystemTrayView::addIcon()
{
}

void FxSystemTrayView::showContextMenu()
{
    if (g_context_menu == nullptr || g_tray_icon == nullptr)
        return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_menu_popup(g_context_menu, nullptr, nullptr,
                   gtk_status_icon_position_menu, g_tray_icon,
                   0, gtk_get_current_event_time());
#pragma GCC diagnostic pop
}

void FxSystemTrayView::addOutputDeviceMenu(PopupMenu* context_menu)
{
    auto selected = FxModel::getModel().getSelectedOutput();

    PopupMenu::Item default_item(selected.deviceFriendlyName.c_str());
    default_item.setID(OUTPUT_MENU_ID_START);
    default_item.setTicked(true);
    context_menu->addItem(default_item);
}

void FxSystemTrayView::showNotification()
{
    notification_.setMessage("Coral is enhancing your audio", {"", ""}, true);
    notification_.showMessage();
}

String FxSystemTrayView::getTruncatedText(const String& text, int max_length)
{
    if (text.length() <= max_length)
        return text;

    return text.substring(0, max_length - 3) + "...";
}

#endif
