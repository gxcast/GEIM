GEIM
====

Gel Electrophoresis Image Manipulate

This is my graduation subject, match the gel pots in eclectrophoresis, so it may be instability, even hazardous, use it carefull. This project is developed in __Code::Blocks__ with __wxWidgets__ library, use __GPLV2__ license.

Project address: <https://github.com/gxcast/GEIM.git>

NOTE
----
* 29.03.2014
    1. add image switch in detect form, "Test Param" and "Batch" valid check
    2. add `wxImgplEvent` type used to notify image panel's parent to syn image display(move zoom et al)

* 28.03.2014
    1. add multiple image open, inner image data alloc and ui dynamic layout

* 27.03.2014
    1. perfect "spot detect form", add some event map
    2. image panel add spot select function

* 26.03.2014
    1. switch branch to dev
    2. add "spot detect form", finish most of the ui layout

* 23.03.2014
    1. fix bug, when iamge's size less than wnd, it's actual position calculate error

* 22.03.2014
    1. Perfect scaling and positon control of image-display module
    2. fix memorydc of double buffer draw exception
    3. finish mouse drag function: iamge move, zoom rect

* 21.03.2014
    1. finish the image dispaly mode, and image zoom/move

