# model-viewer
Simple model viewer for learning purposes

Depdencies:

```
glew
sdl2
sdl2_image
cglm (yay -S cglm) needs glm too
assimp
opengl 4.60
```
Compiling:
```
mkdir build
cd build
cmake ..
make
```
Running:
```
cmd/model_viewer
```
Keys:
```
w - Zoom in
s - Zoom out
<right> - rotate right
<left> - rotate left
<up> - rotate up
<down> rotate down
j - rotate up in z
k - rotate down in z
r - toggle lighting
y, u, i, o, p - Change lighting
q - quit
```
