#include <catch2/catch_amalgamated.hpp>

#include <huxerui/codeeditor.h>

TEST_CASE("TestEditorThemePresetsAreDistinct") {
  const auto light = huxerui::codeeditor::EditorTheme::Default();
  const auto everforest = huxerui::codeeditor::EditorTheme::Everforest();
  REQUIRE_FALSE(light == everforest);
  REQUIRE(light == light);
}

TEST_CASE("TestEditorThemeDerivationMapsTheAmbientColorScheme") {
  // A default-constructed specification carries the ambient palette fields
  // the derivation anchors onto, so the test exercises the mapping contract
  // without pulling SDK theme presets into the link.
  huxerui::ThemeSpec spec;
  const auto derived = huxerui::codeeditor::EditorTheme::FromThemeSpec(spec);

  REQUIRE(derived.background == spec.colors.surface);
  REQUIRE(derived.text_foreground == spec.colors.on_surface);
  REQUIRE_FALSE(derived.text_foreground == derived.background);
}
