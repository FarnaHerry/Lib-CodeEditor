#include <catch2/catch_amalgamated.hpp>

#include <huxerui/codeeditor.h>

// A controller that was never passed to a mounted editor must report the
// documented disconnected behavior for every entry point. This is the
// regression surface for lifecycle bugs where controller state outlives the
// editor: every operation observes plain data and must not dispatch into a
// destroyed extension.
TEST_CASE("TestDisconnectedControllerReportsDisconnected") {
  const huxerui::codeeditor::EditorController controller;
  REQUIRE_FALSE(controller.IsConnected());
}

TEST_CASE("TestDisconnectedControllerOperationsReturnSafely") {
  const huxerui::codeeditor::EditorController controller;

  REQUIRE(controller.Text().empty());
  REQUIRE_FALSE(controller.LoadDocument("document", "text"));
  REQUIRE_FALSE(controller.SetCursor(0, 0));

  REQUIRE_FALSE(controller.RunSearch("pattern"));
  REQUIRE_FALSE(controller.FindNext());
  REQUIRE_FALSE(controller.FindPrevious());
  REQUIRE_FALSE(controller.ReplaceCurrent("replacement"));
  REQUIRE_FALSE(controller.ReplaceAll("replacement"));
  REQUIRE_FALSE(controller.ClearSearch());
  REQUIRE_FALSE(controller.ToggleSearch());

  REQUIRE_FALSE(controller.Cut());
  REQUIRE_FALSE(controller.Copy());
  REQUIRE_FALSE(controller.Paste());
  REQUIRE_FALSE(controller.SelectAll());
}

TEST_CASE("TestControllerEqualityFollowsSharedState") {
  const huxerui::codeeditor::EditorController controller;
  const huxerui::codeeditor::EditorController copy = controller;
  REQUIRE(copy == controller);

  huxerui::codeeditor::EditorController other;
  REQUIRE_FALSE(other == controller);
}
