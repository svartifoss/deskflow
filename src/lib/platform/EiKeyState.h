/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2024 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2022 Red Hat, Inc.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/KeyState.h"
#include "platform/EiComputer.h"

#include <xkbcommon/xkbcommon.h>

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

namespace deskflow {

/// A key state for Ei
class EiKeyState : public KeyState
{
public:
  EiKeyState(EiComputer *computer, IEventQueue *events);
  ~EiKeyState() override;

  void init(int fd, std::size_t len);
  void initDefaultKeymap();

  // IKeyState overrides
  bool fakeCtrlAltDel() override;
  KeyModifierMask pollActiveModifiers() const override;
  std::int32_t pollActiveGroup() const override;
  void pollPressedKeys(KeyButtonSet &pressedKeys) const override;
  KeyID mapKeyFromKeyval(std::uint32_t keyval) const;
  void updateXkbState(std::uint32_t keyval, bool isPressed);
  /**
   * @brief Take the compositor's lock state and active group
   * @param lockedMods the locked modifiers (Caps/Num/Scroll Lock) the compositor reports
   * @param group the layout group the compositor reports as active; out of range for this keymap
   *        leaves the group as it is
   */
  void updateLockedModifiers(xkb_mod_mask_t lockedMods, xkb_layout_index_t group);
  void clearStaleModifiers() override;

protected:
  // KeyState overrides
  void getKeyMap(KeyMap &keyMap) override;
  void fakeKey(const Keystroke &keystroke) override;

private:
  /**
   * @brief Publish the layouts of the current keymap
   *
   * The compositor's keymap names its layouts (xkb_keymap_layout_get_name); their languages
   * replace the layouts guessed at construction, for this key state and for what the app reports
   * to the other side (AppUtil::setKeyboardLayoutList).
   */
  void updateKeyboardLayouts();
  /**
   * @brief Publish the language of the active group (AppUtil::setCurrentLanguageCode)
   *
   * Called whenever the xkb state may have changed its group: after a key, after the
   * compositor's modifier state, after a new keymap.
   */
  void updateCurrentLanguage();
  std::uint32_t convertModMask(xkb_mod_mask_t xkbModMaskIn, bool mapMod2ToNumLock = false) const;
  void assignGeneratedModifiers(std::uint32_t keycode, KeyMap::KeyItem &item);

  EiComputer *m_computer = nullptr;

  xkb_context *m_xkb = nullptr;
  xkb_keymap *m_xkbKeymap = nullptr;
  xkb_state *m_xkbState = nullptr;
  // the ISO 639-1 code of each group of the keymap ("de", "en"), see updateKeyboardLayouts()
  std::vector<std::string> m_groupLanguages;
};

} // namespace deskflow
