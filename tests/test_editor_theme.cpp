#include <catch2/catch_amalgamated.hpp>

#include <huxerui/codeeditor.h>

TEST_CASE("TestEditorThemePresetsAreDistinct") {
  const auto light = huxerui::codeeditor::EditorTheme::Default();
  const auto everforest = huxerui::codeeditor::EditorTheme::Everforest();
  REQUIRE_FALSE(light == everforest);
  REQUIRE(light == light);
}

TEST_CASE("TestEditorThemeDerivesFromAmbientThemeSpec") {
  const auto derived = huxerui::codeeditor::EditorTheme::FromThemeSpec(huxerui::ThemeSpec::Default());
  // Derived themes stay self-consistent: text and background must differ so
  // the editor remains readable on the ambient surface color.
  REQUIRE_FALSE(derived.text_foreground == derived.background);
}
