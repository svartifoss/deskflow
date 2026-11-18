/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "EiKeyStateTests.h"

#include "base/EventQueue.h"
#include "deskflow/AppUtil.h"
#include "deskflow/KeyTypes.h"
#include "platform/EiKeyState.h"

#include <QByteArray>
#include <QTemporaryFile>

#include <cstdint>

namespace {
class TestAppUtil : public AppUtil
{
public:
  int run() override
  {
    return 0;
  }

  std::vector<std::string> getKeyboardLayoutList() override
  {
    return {"en"};
  }

  void setKeyboardLayoutList(const std::vector<std::string> &layouts) override
  {
    reportedLayouts = layouts;
  }

  void setCurrentLanguageCode(const std::string &language) override
  {
    reportedLanguage = language;
  }

  std::string getCurrentLanguageCode() override
  {
    return "en";
  }

  std::vector<std::string> reportedLayouts;
  std::string reportedLanguage;
};

// Two layouts, named as a compositor's compiled keymap names them.
const char TwoLayoutKeymap[] = R"XKB(xkb_keymap {
xkb_keycodes "test" {
    minimum = 8;
    maximum = 255;
    <AD01> = 24;
    <LVL3> = 92;
};
xkb_types "test" {
    type "ONE_LEVEL" {
        modifiers = none;
        level_name[Level1] = "Any";
    };
};
xkb_compat "test" {
    interpret ISO_Next_Group+AnyOfOrNone(all) {
        action = LockGroup(group=+1);
    };
};
xkb_symbols "test" {
    name[Group1] = "German";
    name[Group2] = "English (US)";
    key <AD01> { [ q ], [ q ] };
    key <LVL3> { [ ISO_Next_Group ] };
};
};)XKB";

// XKB keycode of the group switch in TwoLayoutKeymap.
constexpr std::uint32_t NextGroupKeycode = 92;

const char TestKeymap[] = R"XKB(xkb_keymap {
xkb_keycodes "test" {
    minimum = 8;
    maximum = 255;
    <LFSH> = 50;
    <NMLK> = 77;
};
xkb_types "test" {
    type "ONE_LEVEL" {
        modifiers = none;
        level_name[Level1] = "Any";
    };
};
xkb_compat "test" {
    interpret Shift_L+AnyOfOrNone(all) {
        action = SetMods(modifiers=Shift);
    };
    interpret Num_Lock+AnyOfOrNone(all) {
        action = LockMods(modifiers=Mod2);
    };
};
xkb_symbols "test" {
    key <LFSH> { [ Shift_L ] };
    key <NMLK> { [ Num_Lock ] };
    modifier_map Shift { <LFSH> };
    modifier_map Mod2 { <NMLK> };
};
};)XKB";

// XKB keycodes for TestKeymap.
constexpr std::uint32_t LeftShiftKeycode = 50;
constexpr std::uint32_t NumLockKeycode = 77;
} // namespace

void EiKeyStateTests::initTestCase()
{
  m_arch.init();
  m_log.setFilter(LogLevel::Level::Verbose);
}

void EiKeyStateTests::clearStaleModifiers_shiftDownAndNumLockOn_shiftClearedAndNumLockPreserved()
{
  TestAppUtil appUtil;
  EventQueue eventQueue;
  deskflow::EiKeyState keyState(nullptr, &eventQueue);

  QTemporaryFile keymapFile;
  QVERIFY(keymapFile.open());
  const QByteArray keymapData = QByteArray::fromRawData(TestKeymap, sizeof(TestKeymap) - 1);
  QCOMPARE(keymapFile.write(keymapData), keymapData.size());
  QVERIFY(keymapFile.flush());
  keyState.init(keymapFile.handle(), keymapFile.size());

  keyState.updateXkbState(LeftShiftKeycode, true);
  keyState.updateXkbState(NumLockKeycode, true);
  keyState.updateXkbState(NumLockKeycode, false);

  QVERIFY((keyState.pollActiveModifiers() & KeyModifierShift) != 0);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) != 0);

  keyState.clearStaleModifiers();

  QVERIFY((keyState.pollActiveModifiers() & KeyModifierShift) == 0);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) != 0);
}

void EiKeyStateTests::updateLockedModifiers_compositorLockState_numLockFollowsCompositor()
{
  // Real modifier Mod2 is index 4 (Shift, Lock, Control, Mod1, Mod2, ...).
  constexpr std::uint32_t xkbMod2Mask = 1 << 4;

  TestAppUtil appUtil;
  EventQueue eventQueue;
  deskflow::EiKeyState keyState(nullptr, &eventQueue);

  QTemporaryFile keymapFile;
  QVERIFY(keymapFile.open());
  const QByteArray keymapData = QByteArray::fromRawData(TestKeymap, sizeof(TestKeymap) - 1);
  QCOMPARE(keymapFile.write(keymapData), keymapData.size());
  QVERIFY(keymapFile.flush());
  keyState.init(keymapFile.handle(), keymapFile.size());

  keyState.updateXkbState(LeftShiftKeycode, true);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) == 0);

  // NumLock toggled on locally, reported by the compositor
  keyState.updateLockedModifiers(xkbMod2Mask, 0);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) != 0);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierShift) != 0);

  keyState.clearStaleModifiers();
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) != 0);

  // NumLock toggled off locally
  keyState.updateLockedModifiers(0, 0);
  QVERIFY((keyState.pollActiveModifiers() & KeyModifierNumLock) == 0);
}

void EiKeyStateTests::init_keymapWithNamedLayouts_layoutsReportedToAppUtil()
{
#if WINAPI_XWINDOWS
  TestAppUtil appUtil;
  EventQueue eventQueue;
  deskflow::EiKeyState keyState(nullptr, &eventQueue);
  // the default keymap (us) is reported first
  QCOMPARE(appUtil.reportedLayouts, (std::vector<std::string>{"en"}));

  QTemporaryFile keymapFile;
  QVERIFY(keymapFile.open());
  const QByteArray keymapData = QByteArray::fromRawData(TwoLayoutKeymap, sizeof(TwoLayoutKeymap) - 1);
  QCOMPARE(keymapFile.write(keymapData), keymapData.size());
  QVERIFY(keymapFile.flush());
  keyState.init(keymapFile.handle(), keymapFile.size());

  // the compositor's layouts, in layout order, as ISO 639-1 codes
  QCOMPARE(appUtil.reportedLayouts, (std::vector<std::string>{"de", "en"}));
#else
  QSKIP("the layout parser is built with the X11 support");
#endif
}

void EiKeyStateTests::updateXkbState_groupSwitch_activeLanguageReportedToAppUtil()
{
#if WINAPI_XWINDOWS
  TestAppUtil appUtil;
  EventQueue eventQueue;
  deskflow::EiKeyState keyState(nullptr, &eventQueue);

  QTemporaryFile keymapFile;
  QVERIFY(keymapFile.open());
  const QByteArray keymapData = QByteArray::fromRawData(TwoLayoutKeymap, sizeof(TwoLayoutKeymap) - 1);
  QCOMPARE(keymapFile.write(keymapData), keymapData.size());
  QVERIFY(keymapFile.flush());
  keyState.init(keymapFile.handle(), keymapFile.size());
  // the first group is active after a new keymap
  QCOMPARE(appUtil.reportedLanguage, std::string("de"));

  // the group switch (as the compositor's keymap defines it) moves to the second layout
  keyState.updateXkbState(NextGroupKeycode, true);
  keyState.updateXkbState(NextGroupKeycode, false);
  QCOMPARE(appUtil.reportedLanguage, std::string("en"));
  QCOMPARE(keyState.pollActiveGroup(), 1);
#else
  QSKIP("the layout parser is built with the X11 support");
#endif
}

void EiKeyStateTests::updateLockedModifiers_compositorGroup_activeLanguageFollowsCompositor()
{
#if WINAPI_XWINDOWS
  TestAppUtil appUtil;
  EventQueue eventQueue;
  deskflow::EiKeyState keyState(nullptr, &eventQueue);

  QTemporaryFile keymapFile;
  QVERIFY(keymapFile.open());
  const QByteArray keymapData = QByteArray::fromRawData(TwoLayoutKeymap, sizeof(TwoLayoutKeymap) - 1);
  QCOMPARE(keymapFile.write(keymapData), keymapData.size());
  QVERIFY(keymapFile.flush());
  keyState.init(keymapFile.handle(), keymapFile.size());
  QCOMPARE(appUtil.reportedLanguage, std::string("de"));

  // the compositor switched the layout while no key passed through us
  keyState.updateLockedModifiers(0, 1);
  QCOMPARE(appUtil.reportedLanguage, std::string("en"));
  QCOMPARE(keyState.pollActiveGroup(), 1);
  // a group this keymap doesn't have is ignored
  keyState.updateLockedModifiers(0, 7);
  QCOMPARE(appUtil.reportedLanguage, std::string("en"));
  keyState.updateLockedModifiers(0, 0);
  QCOMPARE(appUtil.reportedLanguage, std::string("de"));
#else
  QSKIP("the layout parser is built with the X11 support");
#endif
}

QTEST_MAIN(EiKeyStateTests)
