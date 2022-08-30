---
linkTitle: Sliced Image Type
description: ' Use the sliced image type to intelligently resize parts of your game UI in Open 3D Engine''s UI Editor . '
title: Sliced Image Type
weight: 100
---

O3DE uses slice resizing to resize certain images intelligently. For example, using regular resizing to widen the following image results in distortion of the corners and edges.

![Image with corner detail.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-component-9-sliced-1.png)

![Scaled without slice scaling.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-component-9-sliced-3.gif)

Slice resizing divides an image into nine sections that scale in such a way as to preserve border and corner details. This technique avoids the distortion that occurs with typical image scaling.

Each area is resized as follows:
+ The center is resized both horizontally and vertically
+ The corners aren't resized at all
+ The top and bottom edges are resized horizontally only
+ The right and left edges are resized vertically only

Slice resizing is useful for images with borders and corner details, such as buttons with rounded corners.

![Scaling of each section in slice scaling.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-component-9-sliced-2.png)

![Scaled with slice scaling.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-component-9-sliced-4.gif)

Using the [Sprite Editor](/docs/user-guide/interactivity/user-interface/editor/sprite-editor), you can manipulate where the sections are by dragging the dotted lines on the preview image.

{{< tip >}}
You can see your changes in real time. To do this, before you open the **Sprite Editor**, change the **Image** component's **ImageType** property to **Sliced**.
{{< /tip >}}

![Select Sliced as your ImageType.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-sprite-editor-3.png)

![Preview your sliced image.](/images/user-guide/interactivity/user-interface/editor/sprite-editor/ui-editor-sprite-editor-3.gif)
