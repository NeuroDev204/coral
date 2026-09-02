#!/bin/bash
git checkout fxsound/Source/GUI/FxSystemTrayView.cpp
cat << 'INNER_EOF' >> fxsound/Source/GUI/FxSystemTrayView.cpp

#if !defined(_WIN32)
FxSystemTrayView::FxSystemTrayView() {}
FxSystemTrayView::~FxSystemTrayView() {}
void FxSystemTrayView::modelChanged(FxModel::Event model_event) {}
void FxSystemTrayView::setStatus(bool power, bool processing) {}
Point<int> FxSystemTrayView::getSystemTrayWindowPosition(int width, int height) { return Point<int>(0, 0); }
void FxSystemTrayView::addIcon() {}
void FxSystemTrayView::showContextMenu() {}
void FxSystemTrayView::addOutputDeviceMenu(PopupMenu* context_menu) {}
void FxSystemTrayView::showNotification() {}
String FxSystemTrayView::getTruncatedText(const String& text, int max_length) { return text; }
#endif
INNER_EOF
sed -i -E 's/(#include "\.\.\/Utils\/SysInfo\/SysInfo\.h")/\1\n\n#if defined(_WIN32)/g' fxsound/Source/GUI/FxSystemTrayView.cpp
sed -i -E 's/(#if !defined\(_WIN32\))/#endif\n\n\1/g' fxsound/Source/GUI/FxSystemTrayView.cpp
