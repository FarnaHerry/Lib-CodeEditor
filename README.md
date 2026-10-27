# CodeEditor

A cross-platform code editor for [HuxerUI](https://github.com/HuxerUI/HuxerUI), built on the
[SweetEditor](3dparty/SweetEditor) core. The component is a declarative `View`; documents, editing,
completion, search, decorations, and events are exposed the HuxerUI-native way.

- Header: `<huxerui/codeeditor.h>` — namespace `huxerui::codeeditor`
- CMake target: `codeeditor` (alias `HuxerUI::CodeEditor`)
- Platforms: Windows, macOS, Linux, Web, Android, iOS (following HuxerUI)

## Features

- Declarative component: `CodeEditor(options, controller)` returns an ordinary HuxerUI `View`
- Retained `NodeExtension`: the editor core, document, and input state live on the mounted node
- Editing: caret/selection, undo/redo, auto-indent, auto-closing brackets, Tab/space policy, read-only, pinch zoom
- Custom fonts: `font_family` loads platform-bundled fonts (Android assets) with live switching
- Completion: provider-driven panel, snippet expansion, replacement ranges
- Search and replace: built-in declarative search bar or programmatic `EditorController`
- Full theming: `EditorTheme` covers every visual surface including the syntax token palette
  and rainbow brackets; defaults derive from the ambient HuxerUI theme (light/dark follows
  `MaterialTheme` automatically), and changes apply live without editor state loss
- Decorations: syntax spans, diagnostics, inlay hints, code lens, links, gutter icons, indent guides,
  fold regions, document highlights, bracket matching, phantom text, diff — all through **one
  unified provider interface**
- No built-in highlighting engine: wire SweetLine, a language server, or your own analyzer through
  `EditorDecorationProvider` (the demo shows the optional SweetLine integration)
- Accessibility: the editor exposes a text-field semantic whose value is the document, honoring the
  read-only option; options are validated at composition entry with `HuxerUI`-prefixed diagnostics
- Typed events aggregated in `EditorEvents`

## Quick start

```cpp
#include <huxerui/huxerui.h>
#include <huxerui/codeeditor.h>

using namespace huxerui;

View App() {
  huxerui::codeeditor::EditorOptions options;
  options.initial_text = "int main() {\n  return 0;\n}\n";

  return huxerui::codeeditor::CodeEditor(std::move(options)).With(Grow{});
}

const Application application{
    App,
    {.window = {.title = "CodeEditor", .initial_size = {900.0F, 640.0F}}},
};
```

`document_key` is optional: leave it empty to keep one default document, or set it when switching
between documents so the editor recreates its document state.

For large files, call `PrepareEditorDocument(text)` on a worker and pass the returned
handle in `options.initial_document` (instead of copying `initial_text` into UI declarations).
The editor owns scrolling and viewport rendering. Keep the document handle across remounts
to retain live edits; once mounted, confine the document to the UI thread and never attach
one document to multiple mounted editors. `Controller::LoadDocument(key, text)` explicitly
switches back to the text-loading path. This moves UTF-8 line preparation off the UI thread;
viewport layout and platform font measurement still happen on the UI thread.

## Using the library

Consume the library from any HuxerUI application with `huxerui_use_library`:

```cmake
# In your app's CMakeLists.txt, after huxerui_add_app(...):
huxerui_use_library(your_app
        TARGET HuxerUI::CodeEditor
        PATH "/path/to/Lib-CodeEditor"
)
```

The library links the vendored SweetEditor core itself. Building requires the HuxerUI SDK
(`HUXERUI_HOME`); see the
[HuxerUI installation guide](https://github.com/HuxerUI/HuxerUI/blob/main/docs/guide/installation.md).

## Highlighting: implement a decoration provider

The editor never depends on a highlighting engine. Implement
`EditorDecorationProvider` and return whatever you can compute for the visible range —
syntax spans, diagnostics, inlay hints, code lens, links, gutter icons, indent guides, fold
regions, document highlights, bracket matches, and phantom text:

```cpp
class MyHighlighting final : public huxerui::codeeditor::EditorDecorationProvider {
 public:
  huxerui::codeeditor::EditorDecorationResult ProvideDecorations(
      const huxerui::codeeditor::EditorDecorationContext& context) override {
    huxerui::codeeditor::EditorDecorationResult result;
    result.syntax_spans.emplace_back(
        0, std::vector{{huxerui::codeeditor::StyleSpan{0, 3, huxerui::codeeditor::EditorStyle::Keyword}}}
    );
    return result;
  }
};

options.decoration_providers.push_back(std::make_shared<MyHighlighting>());
```

Style ids come from the `EditorStyle` palette the editor registers by default. For a full
reference — incremental analysis, overscan, folds, rainbow brackets — see the optional SweetLine
integration in [`examples/preview/src/sweetline_provider.cpp`](examples/preview/src/sweetline_provider.cpp).

## Events and controller

```cpp
huxerui::codeeditor::CodeEditor(options, controller)
    .On<huxerui::codeeditor::EditorEvents::TextChanged>([] { /* document changed */ })
    .On<huxerui::codeeditor::EditorEvents::CursorChanged>([](uint32_t line, uint32_t column) {})
    .On<huxerui::codeeditor::EditorEvents::LinkClicked>([](const std::string& url) {});
```

`UseEditorController()` creates scope state for programmatic control — `LoadDocument`,
`Text`, `SetCursor`, search/replace, and `ToggleSearch`. All methods return `false` while no
editor is mounted.

## Repository layout

```text
Lib-CodeEditor/
├── CMakeLists.txt                 # codeeditor library (links SweetEditor only)
├── include/huxerui/codeeditor.h   # public API
├── src/codeeditor/                # component implementation
├── tests/                         # Catch2 suite for pure public-API logic
├── examples/preview/              # demo app; wires optional SweetLine highlighting
│   ├── src/sweetline_provider.*   # reference EditorDecorationProvider
│   ├── platform/android/...assets/fonts/  # Maple Mono (+ no-ligature build)
│   └── platform/                  # per-platform shells
└── 3dparty/SweetEditor/           # editor core (vendored gitlink)
```

Input tracing is disabled by default; opt in with `-DHUXERUI_CODEEDITOR_TRACE=ON` when diagnosing input.

## Building and CI

Desktop, Android, and Termux builds follow the standard HuxerUI flows (`huxerui package <platform>`,
see the [HuxerUI docs](https://github.com/HuxerUI/HuxerUI/tree/main/docs)). The GitHub Actions
workflow ([`.github/workflows/release.yml`](.github/workflows/release.yml)) builds Windows, Linux,
macOS, and Android on every push to `main` and publishes the artifacts to a continuous GitHub
Release; tag pushes (`v*`) publish stable releases.

The Catch2 suite under `tests/` covers options validation, disconnected controller behavior, and
theme derivation. It builds automatically when `HUXERUI_HOME` points at a source HuxerUI checkout
on a desktop host (the Catch2 amalgamation ships with the checkout); force it with
`-DHUXERUI_CODEEDITOR_BUILD_TESTS=ON` and run it through `ctest`.

## Documentation

- [API Reference](docs/codeeditor.md) — full parameter tables, events, decoration interface, controller, and completion types
- [中文 API 参考](docs/codeeditor_zh.md) — 完整参数表、事件、装饰接口、控制器与补全类型
- [Highlighting without SweetLine](docs/highlighting-without-sweetline.md) — split the engine out and plug in your own highlighter

## Third-party dependencies

- [SweetEditor](3dparty/SweetEditor) — cross-platform editor core (vendored gitlink, never modified)
- [SweetLine](3dparty/SweetLine) — highlighting engine, used only by the demo (vendored gitlink)

## License

Component code license is pending; SweetEditor and SweetLine follow their upstream licenses.
