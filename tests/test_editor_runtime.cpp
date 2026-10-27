#include <optional>
#include <string>

#include <catch2/catch_amalgamated.hpp>
#include <huxerui/codeeditor.h>
#include <huxerui/huxerui.h>
#include <huxerui/testing/ui_test.h>

namespace {
using namespace huxerui;
using namespace huxerui::codeeditor;
EditorDocument document;
EditorController controller;
std::shared_ptr<Clipboard> clipboard;
bool read_only = false;
int changes = 0;

View Root() {
  return MaterialTheme(Scope([] {
    clipboard = UseApplication().Clipboard();
    EditorOptions options;
    options.initial_document = document;
    options.read_only = read_only;
    return CodeEditor(std::move(options), controller)
        .On<EditorEvents::TextChanged>([] { ++changes; })
        .With(Grow{})
        .Key("runtime-editor");
  }));
}
const Application application{Root};
}

TEST_CASE("ToolbarClipboardActionsWorkBeforeNativeClipboardCommands") {
  document = PrepareEditorDocument("original\r\nline\n");
  controller = EditorController{};
  read_only = false;
  changes = 0;
  testing::UiTest ui(application, {.viewport = {400, 300}});
  ui.Pump();
  REQUIRE(controller.SelectAll());
  REQUIRE(controller.Copy());
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"original\r\nline\n"});
  REQUIRE(changes == 0);
  REQUIRE(controller.Cut());
  REQUIRE(controller.Text().empty());
  REQUIRE(changes == 1);
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"original\r\nline\n"});
  REQUIRE(clipboard->WriteText("pasted"));
  REQUIRE(controller.Paste());
  REQUIRE(controller.Text() == "pasted");
  REQUIRE(changes == 2);
  const auto editor = ui.Find(testing::UiSelector::Key("runtime-editor"))
      .Find(testing::UiSelector::Type<Canvas>());
  editor.Tap();
  editor.EnterText("x");
  REQUIRE(changes == 3);
}

TEST_CASE("ReadOnlyToolbarRejectsCutAndPasteWithoutChangingDocument") {
  document = PrepareEditorDocument("read-only");
  controller = EditorController{};
  read_only = true;
  changes = 0;
  testing::UiTest ui(application, {.viewport = {400, 300}});
  ui.Pump();
  REQUIRE(controller.SelectAll());
  REQUIRE(controller.Copy());
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"read-only"});
  REQUIRE_FALSE(controller.Cut());
  REQUIRE_FALSE(controller.Paste());
  REQUIRE(controller.Text() == "read-only");
  REQUIRE(changes == 0);
}
