---
title: Add the Open 3D Engine Multiplayer Gem to a Project
description: Add Multiplayer support to an Open 3D Engine project or Gem, including the creation of auto-components.
linkTitle: Project configuration
---

Adding the full functionality of the Open 3D Engine Multiplayer Gem to a project requires making edits to the CMake scripts and source code. These changes enable:

* Linking against the correct core libraries and Gems
* Building [auto-components](./autocomponents)
* Creating and registering component information with the Multiplayer Gem on project start

{{< note >}}
Since both O3DE Gems and projects use the same CMake build functions, these instructions can be used to create a *subgem* of the Multiplayer Gem. This allows you to create a new Gem that extends the behavior of the Multiplayer Gem.
{{< /note >}}

## Build setup

Start by ensuring the Multiplayer Gem is [added to the project](/docs/user-guide/project-config/add-remove-gems/).
After adding the Multiplayer Gem to the project there are more steps that require editing some of the project's CMake and C++ files by hand. 
### CMakeList.txt changes
Make sure the `<ProjectName>.Static` target includes the correct dependencies. 
Find the CMake file defining your project's static target; for example `<ProjectName>/Code/CMakeList.txt`. 
Make the following edits to the `<ProjectName>.Static` target:
1. In the `FILES_CMAKE` section, add `<projectname>_autogen_files.cmake`
    ```cmake
        ly_add_target(
            NAME <ProjectName>.Static STATIC
            ...
            FILES_CMAKE
                ...
                <projectname>_autogen_files.cmake
    ```
    {{< note >}}The `<projectname>_autogen_files.cmake` file is created later in the [Adding AutoGen CMake file](#adding_autogen_file) section. For now, create a reference to it in the `CMakeList.txt`{{< /note >}}

1. In the `BUILD_DEPENDENCIES PUBLIC` section, add `AZ::AzNetworking`, `Gem::Multiplayer`, and `AZ::AzFramework`.
   ```cmake
    ly_add_target(
        NAME <ProjectName>.Static STATIC
        ...
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzFramework
                AZ::AzNetworking
                Gem::Multiplayer
   ```

    {{< note >}}If `BUILD_DEPENDENCIES` does not contain a `PUBLIC` section, add it as shown in the previous code example.{{< /note >}}
1. In the `BUILD_DEPENDENCIES PRIVATE` section, add `Gem::Multiplayer.Static`.
   ```cmake
    ly_add_target(
        NAME <ProjectName>.Static STATIC
        ...
        BUILD_DEPENDENCIES
            PRIVATE
                ...
                Gem::Multiplayer.Static
   ```
1. Also add the following `AUTOGEN_RULES` section to the `<ProjectName>.Static` target:
   
   ```cmake
    ly_add_target(
        NAME <ProjectName>.Static STATIC
        ...
        AUTOGEN_RULES
            *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
            *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
            *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
            *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
   ```

At the end of editing the CMake file, your `<ProjectName>.Static` target should look something like the following:

```cmake
ly_add_target(
    NAME <ProjectName>.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        <projectname>_files.cmake
        <projectname>_autogen_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzFramework
            AZ::AzNetworking
            Gem::Multiplayer
        PRIVATE
            ...
            Gem::Multiplayer.Static
    AUTOGEN_RULES
        *.AutoComponent.xml,AutoComponent_Header.jinja,$path/$fileprefix.AutoComponent.h
        *.AutoComponent.xml,AutoComponent_Source.jinja,$path/$fileprefix.AutoComponent.cpp
        *.AutoComponent.xml,AutoComponentTypes_Header.jinja,$path/AutoComponentTypes.h
        *.AutoComponent.xml,AutoComponentTypes_Source.jinja,$path/AutoComponentTypes.cpp
)
```
<a id="adding_autogen_file"></a>
### Adding AutoGen CMake file
Next, create a new file called `<projectname>_autogen_files.cmake` and place it in the project's code folder. For example: `<ProjectName>/Code/<projectname>_autogen_files.cmake`. The contents of this file add the source templates for [autocomponents](./autocomponents) to the project build.

```cmake
set(FILES
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Common.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Header.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponent_Source.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponentTypes_Header.jinja
    ${LY_ROOT_FOLDER}/Gems/Multiplayer/Code/Include/Multiplayer/AutoGen/AutoComponentTypes_Source.jinja
)
```

### Adding a temporary auto-component
{{< note >}}
You might experience a build failure if multiplayer auto-components are enabled, but no auto-components are created. As a work-around, create a temporary auto-component. Refer to this [issue](https://github.com/o3de/o3de/issues/4058) for more information.
{{< /note >}}
1. Create a new folder under your project's `Code\Source\` directory called `AutoGen`. 
    {{< note >}}This AutoGen directory doesn't have to be temporary. All future multiplayer auto-components can live here.{{< /note >}}
1. Create a new, temporary auto-component file under `Code\Source\AutoGen` called `MyFirstNetworkComponent.AutoComponent.xml`.
    {{< note >}}This guide uses "MyFirstNetworkComponent" as the name for this multiplayer auto-component. You can specify any name for your component, but ensure that the name is used consistently.{{< /note >}}

1. Modify `Code\Source\AutoGen\MyFirstNetworkComponent.AutoComponent.xml` to have the following content:
    ```xml  
    <?xml version="1.0"?>

    <Component
        Name="MyFirstNetworkComponent"
        Namespace="<ProjectName>"
        OverrideComponent="false"
        OverrideController="false"
        OverrideInclude=""
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    </Component>
    ```
    {{< important >}}Make sure that the "Namespace" property is wrapped in quotes and matches the name of your project.
    {{< /important >}}
1. Register the temporary auto-component with CMake by updating `<projectname_files.cmake>`
    ```cmake
    set(FILES
        ...
        Source/AutoGen/MyFirstNetworkComponent.AutoComponent.xml
    )
    ```
{{< note >}}After completing the setup steps in this guide, you can delete the temporary auto-component and create a new auto-component, or use it as a starting point. There must always be at least one auto-component.

You can learn more about multiplayer auto-components [here](/docs/user-guide/gems/reference/multiplayer/multiplayer-gem/autocomponents/), or follow [this tutorial](/docs/learning-guide/tutorials/multiplayer/first-multiplayer-component/).{{< /note >}}

## Module and System component setup

In order to use multiplayer functionality, you need to make small changes to the source code to generate descriptors for multiplayer components and then register these components with the Multiplayer Gem.

### Module.cpp changes

Make the following changes to your project's `Code/Source/<ProjectName>Module.cpp`:
1. Include `AutoComponentTypes.h` at the top of the file.
    ```cpp
    #include <Source/AutoGen/AutoComponentTypes.h>
    ```

1. Edit the `<ProjectName>Module` constructor to create the component descriptors, which allows Multiplayer components to be registered.
    ```cpp
    MultiplayerSampleModule()
        : AZ::Module()
    {
        ...
        CreateComponentDescriptors(m_descriptors); //< Add this line to register your projects multiplayer components
    }
    ```
    {{< important >}}Make sure the call to `CreateComponentDescriptors()` is the *last* line of the constructor.
    {{< /important >}}

### SystemComponent.cpp changes

Make the following changes to your project's `Code/Source/<ProjectName>SystemComponent.cpp` file.
1. Include `AutoComponentTypes.h` at the top of the file.
    ```cpp
    #include <Source/AutoGen/AutoComponentTypes.h>
    ```
2. Register Multiplayer components with the Gem by updating the `Activate()` function.

    ```cpp
    ...
    void <ProjectName>SystemComponent::Activate()
    {
        ...
        RegisterMultiplayerComponents(); //< Register our gems multiplayer components to assign NetComponentIds
    }
    ```

## Rebuild the project
Configuring and building is always required after editing CMake and C++ files. You can use the [Project Manager](/docs/user-guide/project-config/project-manager/) to rebuild, or configure and build via the [command-line interface (CLI)](/docs/user-guide/build/configure-and-build/).