# ImGuiHookFuntion
ImGuiHookFuntion for DirectX12, DirectX11 and DirectX9 (x64/x86)

ImguiHookFunction 用于演示在3d游戏和视频画面上直接绘制imgui界面。

以directx为例,ImguiHookFunction hook directx endscene/present渲染接口,在hook后的新endscene/present渲染接口中绘制imgui界面。

优势:

直接在第3方3d游戏或视频画面上绘制定制imGUI, 不改变3d游戏、视频和其它原生GUI源码结构的情况下定制imGUI。

即使3d游戏或视频处于全屏模式，也可以在画面上直接绘制imGUI。

ImGUI不影响3d游戏或视频窗口的键盘/鼠标消息,也没有抢窗口焦点或焦点切换的问题, ImGUI直接调用opengl/directx 3d接口绘制UI，所以可以无缝融合3d游戏或视频窗口画面。

使用方法(以directx为例):
1.在directx_demo project里的MJEndscene/MJPresent函数内定制imgui代码。
2.第3方3d游戏或视频软件加载directx_demo.dll,directx_demo.dll会自动搜索当前顶层活动窗口,在活动窗口上绘制directx_demo.dll MJEndscene/MJPresent里定制的imgui界面。

Demo:
ImguiHookFunction.sln 演示了使用过程, 运行Hooked_directx9_demo_app，点击加载imgui即可。
https://github.com/MagicJack2024/ImguiHookFunction/wiki
![image](https://github.com/MagicJack2024/ImguiHookFunction/blob/main/demo.gif)   



ImGUI简介
ImGUI主要用于游戏行业，所有的控件都需要手绘实现，当然性能也是满满的，毕竟是直接用dx/opengl来实现。
ImGUI仓库：https://github.com/ocornut/imgui

用于 C++ 的无膨胀图形用户界面库
。它输出优化的顶点缓冲区，您可以在启用 3D 管道的应用程序中随时渲染这些缓冲区。它快速、可移植、与渲染器无关且自包含（无外部依赖项）。

ImGui，它是与平台无关的C++轻量级跨平台图形界面库，没有任何第三方依赖，可以将ImGUI的源码直接加到项目中使用，也可以编译成dll, ImGUI使用DX或者OpenGL进行界面渲染，对于画面质量要求较高，例如客户端游戏，4k/8k视频播放时，用ImGUI是很好的选择，当然，你得非常熟悉DirectX或者OpenGL，不然就是宝剑在手，屠龙无力。相对于Qt、MFC、DuiLib、SOUI等，ImGUI的拓展性更好，也更轻量级


## DirectX12
* Present Hook
* DrawInstanced Hook
* DrawIndexedInstanced Hook
* ExecuteCommandLists Hook
```sh
Supports x64 
```
## DirectX11
* Present Hook
* DrawIndexed Hook
```sh
Supports x86 and x64
```

## DirectX9
* EndScene Hook
* Present Hook
* DrawIndexedPrimitive Hook
* DrawPrimitive Hook
* SetTexture Hook
* Reset Hook
* SetStreamSource Hook
* SetVertexDeclaration Hook
* SetVertexShaderConstantF Hook
* SetVertexShader Hook
* SetPixelShader Hook
```sh
Supports x86 and x64
```

## Built With
* [ImGui v1.90](https://github.com/ocornut/imgui) - Dear ImGui: Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies
* [DirectX Software Development Kit](https://www.microsoft.com/en-us/download/details.aspx?id=6812) - This DirectX SDK release contains updates to tools, utilities, samples, documentation, and runtime debug files for x64 and x86 platforms.

## References
* [ImGui Examples](https://github.com/ocornut/imgui/tree/master/examples)
* [ImGuiHook]https://github.com/furkankadirguzeloglu/ImGuiHook
