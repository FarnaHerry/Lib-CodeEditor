#include <optional>
#include <string>

#include <catch2/catch_amalgamated.hpp>
#include <huxerui/codeeditor.h>
#include <huxerui/huxerui.h>
#include <huxerui/testing/ui_test.h>

namespace {
using namespace huxerui;
using namespace huxerui::codeeditor;
Document document;
EditorController controller;
std::shared_ptr<Clipboard> clipboard;
bool read_only = false;
int changes = 0;

View Root() {
  return MaterialTheme(Scope([] {
    clipboard = UseApplication().Clipboard();
    EditorOptions options;
    options.document = document;
    options.settings.read_only = read_only;
    return CodeEditor(controller, std::move(options))
        .On<EditorEvents::TextChanged>([](const TextChangedEvent&) { ++changes; })
        .With(Grow{})
        .Key("runtime-editor");
  }));
}
const Application application{Root};
}

TEST_CASE("ToolbarClipboardActionsWorkBeforeNativeClipboardCommands") {
  document = Document("original\r\nline\n");
  controller = EditorController{};
  read_only = false;
  changes = 0;
  testing::UiTest ui(application, {.viewport = {400, 300}});
  ui.Pump();
  REQUIRE(controller.SelectAll());
  REQUIRE(controller.CopyToClipboard());
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"original\r\nline\n"});
  REQUIRE(changes == 0);
  REQUIRE(controller.CutToClipboard());
  REQUIRE(document.GetText().empty());
  REQUIRE(changes == 1);
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"original\r\nline\n"});
  REQUIRE(clipboard->WriteText("pasted"));
  REQUIRE(controller.PasteFromClipboard());
  REQUIRE(document.GetText() == "pasted");
  REQUIRE(changes == 2);
  const auto editor = ui.Find(testing::UiSelector::Key("runtime-editor"))
      .Find(testing::UiSelector::Type<Canvas>());
  editor.Tap();
  editor.EnterText("x");
  REQUIRE(changes == 3);
}

TEST_CASE("ReadOnlyToolbarRejectsCutAndPasteWithoutChangingDocument") {
  document = Document("read-only");
  controller = EditorController{};
  read_only = true;
  changes = 0;
  testing::UiTest ui(application, {.viewport = {400, 300}});
  ui.Pump();
  REQUIRE(controller.SelectAll());
  REQUIRE(controller.CopyToClipboard());
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"read-only"});
  REQUIRE_FALSE(controller.CutToClipboard());
  REQUIRE_FALSE(controller.PasteFromClipboard());
  REQUIRE(document.GetText() == "read-only");
  REQUIRE(changes == 0);
}

TEST_CASE("ClipboardShortcutsEditTheFocusedDocument") {
  document = Document("shortcut text");
  controller = EditorController{};
  read_only = false;
  changes = 0;
  testing::UiTest ui(application, {.viewport = {400, 300}});
  ui.Pump();
  ui.Find(testing::UiSelector::Key("runtime-editor"))
      .Find(testing::UiSelector::Type<Canvas>()).Tap();

  const KeyModifiers control{.control = true};
  ui.PressKey(Key::A, control);
  ui.PressKey(Key::C, control);
  REQUIRE(clipboard->ReadText() == std::optional<std::string>{"shortcut text"});
  REQUIRE(changes == 0);
  ui.PressKey(Key::X, control);
  REQUIRE(document.GetText().empty());
  REQUIRE(changes == 1);
  ui.PressKey(Key::V, control);
  REQUIRE(document.GetText() == "shortcut text");
  REQUIRE(changes == 2);
}
