/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/AppUtil.h"

#include <mutex>

#define ARCH_APP_UTIL AppUtilUnix

class IEventQueue;

class AppUtilUnix : public AppUtil
{
public:
  explicit AppUtilUnix(const IEventQueue *);
  ~AppUtilUnix() override = default;

  int run() override;
  std::vector<std::string> getKeyboardLayoutList() override;
  void setKeyboardLayoutList(const std::vector<std::string> &layouts) override;
  std::string getCurrentLanguageCode() override;

private:
  // the layouts of the compositor's keymap (Wayland), see setKeyboardLayoutList()
  std::vector<std::string> m_compositorLayouts;
  std::mutex m_compositorLayoutsMutex;
};
