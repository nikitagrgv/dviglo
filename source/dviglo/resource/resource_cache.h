// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "../containers/hash_set.h"
#include "../io/file.h"
#include "resource.h"

#include <mutex>

namespace dviglo
{

class BackgroundLoader;
class FileWatcher;
class PackageFile;

/// Sets to priority so that a package or file is pushed to the end of the vector.
inline constexpr i32 PRIORITY_LAST = -1;

/// Container of resources with specific type.
struct ResourceGroup
{
    /// Construct with defaults.
    ResourceGroup() :
        memoryBudget_(0),
        memoryUse_(0)
    {
    }

    /// Memory budget.
    unsigned long long memoryBudget_;
    /// Current memory use.
    unsigned long long memoryUse_;
    /// Resources.
    HashMap<StringHash, SharedPtr<Resource>> resources_;
};

/// Resource request types.
enum ResourceRequest
{
    RESOURCE_CHECKEXISTS = 0,
    RESOURCE_GETFILE = 1
};

/// Optional resource request processor. Can deny requests, re-route resource file names, or perform other processing per request.
class DV_API ResourceRouter : public Object
{
public:
    /// Construct.
    explicit ResourceRouter()
    {
    }

    /// Process the resource request and optionally modify the resource name string. Empty name string means the resource is not found or not allowed.
    virtual void Route(String& name, ResourceRequest requestType) = 0;
};

/// %Resource cache subsystem. Loads resources on demand and stores them for later access.
class DV_API ResourceCache : public Object
{
    DV_OBJECT(ResourceCache);

    /// Только Engine может создать и уничтожить объект
    friend class Engine;

private:
    /// Инициализируется в конструкторе
    inline static ResourceCache* instance_ = nullptr;

public:
    static ResourceCache* instance() { return instance_; }

private:
    /// Construct.
    explicit ResourceCache();
    /// Destruct. Free all resources.
    ~ResourceCache() override;

public:
    // Запрещаем копирование
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator =(const ResourceCache&) = delete;

    /// Add a resource load directory. Optional priority parameter which will control search order.
    bool add_resource_dir(const String& pathName, i32 priority = PRIORITY_LAST);

    /// Add a package file for loading resources from. Optional priority parameter which will control search order.
    bool add_package_file(PackageFile* package, i32 priority = PRIORITY_LAST);

    /// Add a package file for loading resources from by name. Optional priority parameter which will control search order.
    bool add_package_file(const String& fileName, i32 priority = PRIORITY_LAST);

    /// Add a manually created resource. Must be uniquely named within its type.
    bool add_manual_resource(Resource* resource);

    /// Remove a resource load directory.
    void remove_resource_dir(const String& pathName);

    /// Remove a package file. Optionally release the resources loaded from it.
    void remove_package_file(PackageFile* package, bool releaseResources = true, bool forceRelease = false);

    /// Remove a package file by name. Optionally release the resources loaded from it.
    void remove_package_file(const String& fileName, bool releaseResources = true, bool forceRelease = false);

    /// Release a resource by name.
    void release_resource(StringHash type, const String& name, bool force = false);

    /// Release all resources of a specific type.
    void release_resources(StringHash type, bool force = false);

    /// Release resources of a specific type and partial name.
    void release_resources(StringHash type, const String& partialName, bool force = false);

    /// Release resources of all types by partial name.
    void release_resources(const String& partialName, bool force = false);

    /// Release all resources. When called with the force flag false, releases all currently unused resources.
    void release_all_resources(bool force = false);

    /// Reload a resource. Return true on success. The resource will not be removed from the cache in case of failure.
    bool reload_resource(Resource* resource);

    /// Reload a resource based on filename. Causes also reload of dependent resources if necessary.
    void reload_resource_with_dependencies(const String& fileName);

    /// Set memory budget for a specific resource type, default 0 is unlimited.
    void SetMemoryBudget(StringHash type, unsigned long long budget);

    /// Enable or disable automatic reloading of resources as files are modified. Default false.
    void SetAutoReloadResources(bool enable);

    /// Enable or disable returning resources that failed to load. Default false. This may be useful in editing to not lose resource ref attributes.
    void SetReturnFailedResources(bool enable) { returnFailedResources_ = enable; }

    /// Define whether when getting resources should check package files or directories first. True for packages, false for directories.
    void SetSearchPackagesFirst(bool value) { searchPackagesFirst_ = value; }

    /// Set how many milliseconds maximum per frame to spend on finishing background loaded resources.
    void SetFinishBackgroundResourcesMs(int ms) { finishBackgroundResourcesMs_ = Max(ms, 1); }

    /// Add a resource router object. By default there is none, so the routing process is skipped.
    void add_resource_router(ResourceRouter* router, bool addAsFirst = false);

    /// Remove a resource router object.
    void remove_resource_router(ResourceRouter* router);

    /// Open and return a file from the resource load paths or from inside a package file. If not found, use a fallback search with absolute path. Return null if fails. Can be called from outside the main thread.
    std::shared_ptr<File> GetFile(const String& name, bool sendEventOnFailure = true);

    /// Return a resource by type and name. Load if not loaded yet. Return null if not found or if fails, unless SetReturnFailedResources(true) has been called. Can be called only from the main thread.
    Resource* GetResource(StringHash type, const String& name, bool sendEventOnFailure = true);

    /// Load a resource without storing it in the resource cache. Return null if not found or if fails. Can be called from outside the main thread if the resource itself is safe to load completely (it does not possess for example GPU data).
    SharedPtr<Resource> GetTempResource(StringHash type, const String& name, bool sendEventOnFailure = true);

    /// Background load a resource. An event will be sent when complete. Return true if successfully stored to the load queue, false if eg. already exists. Can be called from outside the main thread.
    bool background_load_resource(StringHash type, const String& name, bool sendEventOnFailure = true, Resource* caller = nullptr);

    /// Return number of pending background-loaded resources.
    unsigned GetNumBackgroundLoadResources() const;

    /// Return all loaded resources of a specific type.
    void GetResources(Vector<Resource*>& result, StringHash type) const;

    /// Return an already loaded resource of specific type & name, or null if not found. Will not load if does not exist.
    Resource* GetExistingResource(StringHash type, const String& name);

    /// Return all loaded resources.
    const HashMap<StringHash, ResourceGroup>& GetAllResources() const { return resourceGroups_; }

    /// Return added resource load directories.
    const Vector<String>& GetResourceDirs() const { return resourceDirs_; }

    /// Return added package files.
    const Vector<SharedPtr<PackageFile>>& GetPackageFiles() const { return packages_; }

    /// Template version of returning a resource by name.
    template <class T> T* GetResource(const String& name, bool sendEventOnFailure = true);

    /// Template version of returning an existing resource by name.
    template <class T> T* GetExistingResource(const String& name);

    /// Template version of loading a resource without storing it to the cache.
    template <class T> SharedPtr<T> GetTempResource(const String& name, bool sendEventOnFailure = true);

    /// Template version of releasing a resource by name.
    template <class T> void release_resource(const String& name, bool force = false);

    /// Template version of queueing a resource background load.
    template <class T> bool background_load_resource(const String& name, bool sendEventOnFailure = true, Resource* caller = nullptr);

    /// Template version of returning loaded resources of a specific type.
    template <class T> void GetResources(Vector<T*>& result) const;

    /// Return whether a file exists in the resource directories or package files. Does not check manually added in-memory resources.
    bool Exists(const String& name) const;

    /// Return memory budget for a resource type.
    unsigned long long GetMemoryBudget(StringHash type) const;

    /// Return total memory use for a resource type.
    unsigned long long GetMemoryUse(StringHash type) const;

    /// Return total memory use for all resources.
    unsigned long long GetTotalMemoryUse() const;

    /// Return full absolute file name of resource if possible, or empty if not found.
    String GetResourceFileName(const String& name) const;

    /// Return whether automatic resource reloading is enabled.
    bool GetAutoReloadResources() const { return autoReloadResources_; }

    /// Return whether resources that failed to load are returned.
    bool GetReturnFailedResources() const { return returnFailedResources_; }

    /// Return whether when getting resources should check package files or directories first.
    bool GetSearchPackagesFirst() const { return searchPackagesFirst_; }

    /// Return how many milliseconds maximum to spend on finishing background loaded resources.
    int GetFinishBackgroundResourcesMs() const { return finishBackgroundResourcesMs_; }

    /// Return a resource router by index.
    ResourceRouter* GetResourceRouter(unsigned index) const;

    /// Return either the path itself or its parent, based on which of them has recognized resource subdirectories.
    String GetPreferredResourceDir(const String& path) const;

    /// Remove unsupported constructs from the resource name to prevent ambiguity, and normalize absolute filename to resource path relative if possible.
    String sanitate_resource_name(const String& name) const;

    /// Remove unnecessary constructs from a resource directory name and ensure it to be an absolute path.
    String sanitate_resource_dir_name(const String& name) const;

    /// Store a dependency for a resource. If a dependency file changes, the resource will be reloaded.
    void store_resource_dependency(Resource* resource, const String& dependency);

    /// Reset dependencies for a resource.
    void reset_dependencies(Resource* resource);

    /// Returns a formatted string containing the memory actively used.
    String print_memory_usage() const;

private:
    /// Find a resource.
    const SharedPtr<Resource>& find_resource(StringHash type, StringHash nameHash);
    /// Find a resource by name only. Searches all type groups.
    const SharedPtr<Resource>& find_resource(StringHash nameHash);
    /// Release resources loaded from a package file.
    void release_package_resources(PackageFile* package, bool force = false);
    /// Update a resource group. Recalculate memory use and release resources if over memory budget.
    void update_resource_group(StringHash type);
    /// Handle begin frame event. Automatic resource reloads and the finalization of background loaded resources are processed here.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    /// Search FileSystem for file.
    File* search_resource_dirs(const String& name);
    /// Search resource packages for file.
    File* search_packages(const String& name);

    /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
    mutable std::mutex resource_mutex_;
    /// Resources by type.
    HashMap<StringHash, ResourceGroup> resourceGroups_;
    /// Resource load directories.
    Vector<String> resourceDirs_;
    /// File watchers for resource directories, if automatic reloading enabled.
    Vector<SharedPtr<FileWatcher>> fileWatchers_;
    /// Package files.
    Vector<SharedPtr<PackageFile>> packages_;
    /// Dependent resources. Only used with automatic reload to eg. trigger reload of a cube texture when any of its faces change.
    HashMap<StringHash, HashSet<StringHash>> dependentResources_;
    /// Resource background loader.
    SharedPtr<BackgroundLoader> backgroundLoader_;
    /// Resource routers.
    Vector<SharedPtr<ResourceRouter>> resourceRouters_;
    /// Automatic resource reloading flag.
    bool autoReloadResources_;
    /// Return failed resources flag.
    bool returnFailedResources_;
    /// Search priority flag.
    bool searchPackagesFirst_;
    /// Resource routing flag to prevent endless recursion.
    mutable bool isRouting_;
    /// How many milliseconds maximum per frame to spend on finishing background loaded resources.
    int finishBackgroundResourcesMs_;
};

#define DV_RES_CACHE (dviglo::ResourceCache::instance())

template <class T> T* ResourceCache::GetExistingResource(const String& name)
{
    StringHash type = T::GetTypeStatic();
    return static_cast<T*>(GetExistingResource(type, name));
}

template <class T> T* ResourceCache::GetResource(const String& name, bool sendEventOnFailure)
{
    StringHash type = T::GetTypeStatic();
    return static_cast<T*>(GetResource(type, name, sendEventOnFailure));
}

template <class T> void ResourceCache::release_resource(const String& name, bool force)
{
    StringHash type = T::GetTypeStatic();
    release_resource(type, name, force);
}

template <class T> SharedPtr<T> ResourceCache::GetTempResource(const String& name, bool sendEventOnFailure)
{
    StringHash type = T::GetTypeStatic();
    return StaticCast<T>(GetTempResource(type, name, sendEventOnFailure));
}

template <class T> bool ResourceCache::background_load_resource(const String& name, bool sendEventOnFailure, Resource* caller)
{
    StringHash type = T::GetTypeStatic();
    return background_load_resource(type, name, sendEventOnFailure, caller);
}

template <class T> void ResourceCache::GetResources(Vector<T*>& result) const
{
    Vector<Resource*>& resources = reinterpret_cast<Vector<Resource*>&>(result);
    StringHash type = T::GetTypeStatic();
    GetResources(resources, type);

    // Perform conversion of the returned pointers
    for (i32 i = 0; i < result.Size(); ++i)
    {
        Resource* resource = resources[i];
        result[i] = static_cast<T*>(resource);
    }
}

/// Register Resource library subsystems and objects.
void DV_API register_resource_library();

}
