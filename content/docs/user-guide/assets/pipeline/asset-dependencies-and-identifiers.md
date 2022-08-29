---
linkTitle: Asset Dependencies and Identifiers 
title: Asset Dependencies and Identifiers 
description: Asset dependencies and identifiers ensure that asset references can be met when assets are processed, loaded, and packaged.
weight: 500
toc: true
---

Both source and product assets might have dependencies. Source asset and job dependencies are inputs to the **Asset Pipeline** and asset processing jobs. Product dependencies are outputs from the Asset Pipeline and are used to load and package product assets. Asset identifiers help ensure both types of asset dependencies are met.

## Source dependencies

Source dependencies are a process time concept. This means that this dependency type is used when processing assets for your project, source dependencies are not used at runtime.

A source dependency triggers **Asset Processor** to rerun the job process step whenever the file declared as a source dependency changes. For example, image files declare a source dependency on the `.preset` file containing the preset information used to process the image, so if the preset changes the image will be reprocessed and use the new preset settings.

## Job dependencies

Job dependencies are a process time concept.

A job dependency is used when a job must read the product asset output of another job. A job dependency is declared against another job. When a job that has a job dependency is run, it first ensures that all of its job dependencies are run. If a job that is a job depenedency is run, it will trigger any jobs that depend on it to run.

The shader and material relationship is an example of a job dependency. The material cannot be processed until the shader job is completed because the material requires information from product assets of the shader job. If the source shader asset is changed, causing the shader job to run, then the material job with that job dependency will run after the shader job finishes.

Required job dependencies cause the job to fail if the required product asset is not available.

Jobs can generate multiple product assets, but a job might not need all product assets from another job that was declared as a job dependency. In this scenario, the required product assets are specified for the job dependency. If the hash of any of the specified product assets changes, the job with the dependency is run.

A material, for example, has a job dependency on a shader. The shader job generates multiple product assets including assets that contain shader logic and a shader configuration. The material job dependency only needs the configuration from the shader, not the logic. The material job runs when the shader configuration is changed, not when the shader logic is changed.

In some cases, a builder requires the output of another builder in order to process the asset it is working on.  If the other builder only outputs a single product, a regular job dependency works just fine.  Sometimes, however, the other builder outputs many products, some of which might not be updated every time the builder runs.  This can result in Asset Processor doing unnecessary work to rebuild dependent assets when their dependencies haven't actually changed.  For these cases, builders can specify the exact set of products they depend on by providing a list of SubIds in the AssetBuilderSDK::JobDependency::m_productSubIds field.  When a builder provides a list of specific products to depend on, Asset Processor will only trigger the dependency when the hash of the contents of any of the specified products changes.

## Product dependencies

Product dependencies are a packaging and runtime concept. This means these dependencies are not used during processing of assets, but they are declared there.

Product dependencies contain data on the relationships between product assets. Product dependencies are used during runtime loading and asset bundling.

When a product asset is loaded at runtime, the product dependencies specify other product assets that must be loaded for this product asset to function. Similarly, when bundling assets, the product dependencies specify additional product assets required by the assets being bundled. An easy to understand example is a level prefab. All the meshes, shaders, materials, audio files, and scripts used by entities and prefabs that are placed in the level are dependencies of the level prefab. When you load or bundle a level prefab, the chain of asset dependencies specifies all the product assets required by the level prefab, as well as the product assets referenced by those dependencies, and so on, until all the product dependencies are met.

Product dependencies can also be marked as required or optional. If a required product dependency is not available, the runtime load process or the bundling process fails. If an optional product dependency is not available, the runtime load process or bundling process can proceed.

## Asset identifiers

Source assets are identified by a Universally Unique Identifier (UUID). This is also referenced as a Globally Unique Identifier (GUID) in some parts of O3DE. The source asset UUID is generated based on the file name and scan directory relative path. This ensures the source asset UUID is identical on any host platform machine for the project.

Product assets are identified by a combination of the UUID for the source asset and a sub ID defined by the **Asset Builder** that produced the product asset. Builder sub IDs must be deterministic and unique, so that references to the product assets can be maintained when assets are updated. When authoring a builder, keep the following in mind:

* Other builders may process the same source asset and also generate products. This is an area where sub ID collisions can occur.
* Keep sub IDs as consistent as possible for product assets based on changes to a source asset.

Downstream references, such as a prefab file referencing a product asset via asset ID, break if the sub ID doesn't resolve, or resolves to a different product asset. In the rare event of a product asset ID collision, Asset Processor notifies you that an asset already exists with the product ID so that you can resolve the issue.

{{< note >}}
Product assets are unique across target platforms. The differences for each target platform most often are platform performance optimizations.

If you author a builder that outputs product assets for different target platforms, you should consider how the product assets are accessed at runtime. It's recommended to use the same sub ID for similar assets for each target platform. For example, the source asset `_dev_purple.tif` generates a streamingimage file `_dev_purple.tif.streamingimage` , which has the sub ID 1000, for all platforms. This means that any other content that references `_dev_purple.tif.streamingimage` can reference the sub ID and get the same functional image on each target platform.
{{< /note >}}
