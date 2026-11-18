/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#if WINAPI_XWINDOWS
#pragma once

#include <string>
#include <vector>

class XkbLayoutsParser
{
public:
  static std::vector<std::string> getXkbLanguageList();
  static std::string convertLayoutToISO(const std::string &layoutLangCode);

  /**
   * @brief Language codes for layouts given by their description
   *
   * A compiled keymap (as a Wayland compositor hands it to its clients) names its layouts by the
   * descriptions of xkeyboard-config ("German", "English (US)", "German (no dead keys)"), not by
   * the layout and variant names the X rules use. This resolves those descriptions through the
   * xkb registry to the same ISO 639-1 codes getXkbLanguageList() produces.
   * @param layoutDescriptions the keymap's layout names, in layout order
   * @return ISO 639-1 codes, unique, in layout order; unknown descriptions are skipped
   */
  static std::vector<std::string>
  getLanguageListForLayoutDescriptions(const std::vector<std::string> &layoutDescriptions);

private:
  struct Lang
  {
    std::string name = "";
    std::string description = "";
    std::vector<std::string> layoutBaseISO639_2;
    std::vector<Lang> variants;
  };

  static std::vector<Lang> getAllLanguageData();

  static void appendVectorUniq(const std::vector<std::string> &source, std::vector<std::string> &dst);

  static void convertLayoutToISO639_2(
      const std::vector<std::string> &layoutNames, const std::vector<std::string> &layoutVariantNames,
      std::vector<std::string> &iso639_2Codes
  );

  static std::vector<std::string> convertISO639_2ToISO639_1(const std::vector<std::string> &iso639_2Codes);
};

#endif // WINAPI_XWINDOWS
