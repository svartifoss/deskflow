/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/DeskflowException.h"
#include "deskflow/IApp.h"

#include <string>
#include <vector>

class AppUtil
{
public:
  AppUtil();
  virtual ~AppUtil() = default;

  void adoptApp(IApp *app);
  IApp &app() const;
  virtual void exitApp(int code)
  {
    throw ExitAppException(code);
  }

  static AppUtil &instance();
  static void exitAppStatic(int code)
  {
    instance().exitApp(code);
  }

  // Virtual Methods subclasses can impliment
  virtual int run() = 0;
  virtual std::vector<std::string> getKeyboardLayoutList() = 0;

  /**
   * @brief Remember the keyboard layouts the platform learnt from the compositor's keymap.
   *
   * On Wayland the layout list cannot be read from the display server; the platform code learns
   * it from the keymap the compositor hands over (see EiKeyState) and reports it here, so that
   * getKeyboardLayoutList() returns the compositor's layouts instead of an X11 fallback.
   * @param layouts ISO 639-1 language codes, in layout order
   */
  virtual void setKeyboardLayoutList(const std::vector<std::string> &layouts)
  {
    // platforms that read their layouts directly ignore this
  }
  virtual std::string getCurrentLanguageCode() = 0;

private:
  IApp *m_app = nullptr;
  static AppUtil *s_instance;
};
