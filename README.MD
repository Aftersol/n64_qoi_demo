# N64 QOI Viewer

An image viewer that loads QOI images on N64

![An fuzzy image of a parrot](demo_images/qoi_demo_image0.jpg)
![An fuzzy image of four dice. Red die is in front. Blue, green, yellow in the back in a triangle clockwise formation](demo_images/qoi_demo_image1.jpg)

Make sure you are on the preview branch of libdragon.

Clone this repository with `--recurse-submodules` or if you haven't run:

```bash
git submodule update --init
```

To initalize libdragon:
```bash
libdragon init
```
To build this project:

```bash
libdragon make
```

---

## Licenses

Everything in the src folder is licensed under MIT License

In the filesystem folder the following images are in public domain:
- qrcode.qoi
- smpte_color_bars.qoi
- ebu_colour_bars.qoi

The rest of the images in the filesystem folder are under their respective licenses