/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2014 - 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "XkbLayoutParserTests.h"

#include "deskflow/unix/XkbLayoutsParser.h"

void XkbLayoutParserTests::convertLayouts()
{
  // An empty layout name yields an empty result.
  QCOMPARE(XkbLayoutsParser::convertLayoutToISO(""), "");

  // An unknown layout name is not present in the registry.
  QCOMPARE(XkbLayoutsParser::convertLayoutToISO("notARealLayout"), "");

  // A well-known layout resolves to its ISO 639-1 code. "us" is always
  // present in the xkeyboard-config data and maps to "eng" -> "en".
  QCOMPARE(XkbLayoutsParser::convertLayoutToISO("us"), "en");
}

void XkbLayoutParserTests::convertLayoutDescriptions()
{
  using Strings = std::vector<std::string>;
  // Descriptions are what a compiled keymap carries as layout names (xkb_keymap_layout_get_name);
  // "English (US)" and "German" are always present in the xkeyboard-config data.
  QCOMPARE(XkbLayoutsParser::getLanguageListForLayoutDescriptions({}), Strings{});
  QCOMPARE(XkbLayoutsParser::getLanguageListForLayoutDescriptions({""}), Strings{});
  QCOMPARE(XkbLayoutsParser::getLanguageListForLayoutDescriptions({"not a real layout"}), Strings{});
  QCOMPARE(XkbLayoutsParser::getLanguageListForLayoutDescriptions({"English (US)"}), Strings{"en"});
  QCOMPARE(XkbLayoutsParser::getLanguageListForLayoutDescriptions({"German"}), Strings{"de"});
  // a variant resolves like its layout, the order is the layout order, duplicates collapse
  QCOMPARE(
      XkbLayoutsParser::getLanguageListForLayoutDescriptions({"German (no dead keys)", "English (US)", "German"}),
      (Strings{"de", "en"})
  );
}

QTEST_MAIN(XkbLayoutParserTests)
