// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "../containers/hash_set.h"
#include "../resource/xml_element.h"
#include "../resource/json_file.h"
#include "node.h"
#include "scene_resolver.h"

#include <memory>
#include <mutex>


namespace dviglo
{

class File;
class PackageFile;

inline constexpr id32 FIRST_REPLICATED_ID = 0x1;
inline constexpr id32 LAST_REPLICATED_ID = 0xffffff;
inline constexpr id32 FIRST_LOCAL_ID = 0x01000000;
inline constexpr id32 LAST_LOCAL_ID = 0xffffffff;

/// Asynchronous scene loading mode.
enum LoadMode
{
    /// Preload resources used by a scene or object prefab file, but do not load any scene content.
    LOAD_RESOURCES_ONLY = 0,
    /// Load scene content without preloading. Resources will be requested synchronously when encountered.
    LOAD_SCENE,
    /// Default mode: preload resources used by the scene first, then load the scene content.
    LOAD_SCENE_AND_RESOURCES
};

/// Asynchronous loading progress of a scene.
struct AsyncProgress
{
    /// File for binary mode.
    SharedPtr<File> file_;
    /// XML file for XML mode.
    SharedPtr<XmlFile> xmlFile_;
    /// JSON file for JSON mode.
    SharedPtr<JSONFile> jsonFile_;

    /// Current XML element for XML mode.
    XmlElement xmlElement_;

    /// Current JSON child array and for JSON mode.
    i32 jsonIndex_;

    /// Current load mode.
    LoadMode mode_;
    /// Resource name hashes left to load.
    HashSet<StringHash> resources_;
    /// Loaded resources.
    i32 loadedResources_;
    /// Total resources.
    i32 totalResources_;
    /// Loaded root-level nodes.
    i32 loadedNodes_;
    /// Total root-level nodes.
    i32 totalNodes_;
};

/// Root scene node, represents the whole scene.
class DV_API Scene : public Node
{
    DV_OBJECT(Scene);

public:
    using Node::GetComponent;
    using Node::save_xml;
    using Node::save_json;

    Signal<Scene*, float> scene_update;
    Signal<Scene*, float> scene_post_update;

    /// Construct.
    explicit Scene();
    /// Destruct.
    ~Scene() override;
    /// Register object factory. Node must be registered first.
    static void register_object();

    /// Load from binary data. Removes all existing child nodes and components first. Return true if successful.
    bool Load(Deserializer& source) override;
    /// Save to binary data. Return true if successful.
    bool Save(Serializer& dest) const override;
    /// Load from XML data. Removes all existing child nodes and components first. Return true if successful.
    bool load_xml(const XmlElement& source) override;
    /// Load from JSON data. Removes all existing child nodes and components first. Return true if successful.
    bool load_json(const JSONValue& source) override;
    /// Mark for attribute check on the next network update.
    void MarkNetworkUpdate() override;
    /// Add a replication state that is tracking this scene.
    void AddReplicationState(NodeReplicationState* state) override;

    /// Load from an XML file. Return true if successful.
    bool load_xml(Deserializer& source);
    /// Load from a JSON file. Return true if successful.
    bool load_json(Deserializer& source);
    /// Save to an XML file. Return true if successful.
    bool save_xml(Serializer& dest, const String& indentation = "\t") const;
    /// Save to a JSON file. Return true if successful.
    bool save_json(Serializer& dest, const String& indentation = "\t") const;
    /// Load from a binary file asynchronously. Return true if started successfully. The LOAD_RESOURCES_ONLY mode can also be used to preload resources from object prefab files.
    bool LoadAsync(File* file, LoadMode mode = LOAD_SCENE_AND_RESOURCES);
    /// Load from an XML file asynchronously. Return true if started successfully. The LOAD_RESOURCES_ONLY mode can also be used to preload resources from object prefab files.
    bool LoadAsyncXML(File* file, LoadMode mode = LOAD_SCENE_AND_RESOURCES);
    /// Load from a JSON file asynchronously. Return true if started successfully. The LOAD_RESOURCES_ONLY mode can also be used to preload resources from object prefab files.
    bool LoadAsyncJSON(File* file, LoadMode mode = LOAD_SCENE_AND_RESOURCES);
    /// Stop asynchronous loading.
    void StopAsyncLoading();
    /// Instantiate scene content from binary data. Return root node if successful.
    Node* Instantiate(Deserializer& source, const Vector3& position, const Quaternion& rotation, CreateMode mode = REPLICATED);
    /// Instantiate scene content from XML data. Return root node if successful.
    Node* InstantiateXML
        (const XmlElement& source, const Vector3& position, const Quaternion& rotation, CreateMode mode = REPLICATED);
    /// Instantiate scene content from XML data. Return root node if successful.
    Node* InstantiateXML(Deserializer& source, const Vector3& position, const Quaternion& rotation, CreateMode mode = REPLICATED);
    /// Instantiate scene content from JSON data. Return root node if successful.
    Node* InstantiateJSON
        (const JSONValue& source, const Vector3& position, const Quaternion& rotation, CreateMode mode = REPLICATED);
    /// Instantiate scene content from JSON data. Return root node if successful.
    Node* InstantiateJSON(Deserializer& source, const Vector3& position, const Quaternion& rotation, CreateMode mode = REPLICATED);

    /// Clear scene completely of either replicated, local or all nodes and components.
    void Clear(bool clearReplicated = true, bool clearLocal = true);
    /// Enable or disable scene update.
    void SetUpdateEnabled(bool enable);
    /// Set update time scale. 1.0 = real time (default).
    void SetTimeScale(float scale);
    /// Set elapsed time in seconds. This can be used to prevent inaccuracy in the timer if the scene runs for a long time.
    void SetElapsedTime(float time);
    /// Set network client motion smoothing constant.
    void SetSmoothingConstant(float constant);
    /// Set network client motion smoothing snap threshold.
    void SetSnapThreshold(float threshold);
    /// Set maximum milliseconds per frame to spend on async scene loading.
    void SetAsyncLoadingMs(int ms);
    /// Add a required package file for networking. To be called on the server.
    void AddRequiredPackageFile(PackageFile* package);
    /// Clear required package files.
    void ClearRequiredPackageFiles();
    /// Register a node user variable hash reverse mapping (for editing).
    void RegisterVar(const String& name);
    /// Unregister a node user variable hash reverse mapping.
    void UnregisterVar(const String& name);
    /// Clear all registered node user variable hash reverse mappings.
    void UnregisterAllVars();

    /// Return node from the whole scene by ID, or null if not found.
    Node* GetNode(NodeId id) const;
    /// Return component from the whole scene by ID, or null if not found.
    Component* GetComponent(ComponentId id) const;
    /// Get nodes with specific tag from the whole scene, return false if empty.
    bool GetNodesWithTag(Vector<Node*>& dest, const String& tag)  const;

    /// Return whether updates are enabled.
    bool IsUpdateEnabled() const { return updateEnabled_; }

    /// Return whether an asynchronous loading operation is in progress.
    bool IsAsyncLoading() const { return asyncLoading_; }

    /// Return asynchronous loading progress between 0.0 and 1.0, or 1.0 if not in progress.
    float GetAsyncProgress() const;

    /// Return the load mode of the current asynchronous loading operation.
    LoadMode GetAsyncLoadMode() const { return asyncProgress_.mode_; }

    /// Return source file name.
    const String& GetFileName() const { return fileName_; }

    /// Return source file checksum.
    hash32 GetChecksum() const { return checksum_; }

    /// Return update time scale.
    float GetTimeScale() const { return timeScale_; }

    /// Return elapsed time in seconds.
    float GetElapsedTime() const { return elapsedTime_; }

    /// Return motion smoothing constant.
    float smoothing_constant() const { return smoothing_constant_; }

    /// Return motion smoothing snap threshold.
    float snap_threshold() const { return snap_threshold_; }

    /// Return maximum milliseconds per frame to spend on async loading.
    int GetAsyncLoadingMs() const { return asyncLoadingMs_; }

    /// Return required package files.
    const Vector<SharedPtr<PackageFile>>& GetRequiredPackageFiles() const { return requiredPackageFiles_; }

    /// Return a node user variable name, or empty if not registered.
    const String& GetVarName(StringHash hash) const;

    /// Update scene. Called by handle_update.
    void Update(float timeStep);
    /// Begin a threaded update. During threaded update components can choose to delay dirty processing.
    void BeginThreadedUpdate();
    /// End a threaded update. Notify components that marked themselves for delayed dirty processing.
    void EndThreadedUpdate();
    /// Add a component to the delayed dirty notify queue. Is thread-safe.
    void DelayedMarkedDirty(Component* component);

    /// Return threaded update flag.
    bool IsThreadedUpdate() const { return threadedUpdate_; }

    /// Get free node ID, either non-local or local.
    NodeId GetFreeNodeID(CreateMode mode);
    /// Get free component ID, either non-local or local.
    ComponentId GetFreeComponentID(CreateMode mode);
    /// Return whether the specified id is a replicated id.
    static bool IsReplicatedID(id32 id) { return id < FIRST_LOCAL_ID; }

    /// Cache node by tag if tag not zero, no checking if already added. Used internaly in Node::add_tag.
    void NodeTagAdded(Node* node, const String& tag);
    /// Cache node by tag if tag not zero.
    void NodeTagRemoved(Node* node, const String& tag);

    /// Node added. Assign scene pointer and add to ID map.
    void NodeAdded(Node* node);
    /// Node removed. Remove from ID map.
    void NodeRemoved(Node* node);
    /// Component added. Add to ID map.
    void ComponentAdded(Component* component);
    /// Component removed. Remove from ID map.
    void ComponentRemoved(Component* component);
    /// Set node user variable reverse mappings.
    void SetVarNamesAttr(const String& value);
    /// Return node user variable reverse mappings.
    String GetVarNamesAttr() const;
    /// Prepare network update by comparing attributes and marking replication states dirty as necessary.
    void PrepareNetworkUpdate();
    /// Clean up all references to a network connection that is about to be removed.
    void CleanupConnection(Connection* connection);
    /// Mark a node for attribute check on the next network update.
    void MarkNetworkUpdate(Node* node);
    /// Mark a component for attribute check on the next network update.
    void MarkNetworkUpdate(Component* component);
    /// Mark a node dirty in scene replication states. The node does not need to have own replication state yet.
    void MarkReplicationDirty(Node* node);

private:
    /// Handle the logic update event to update the scene, if active.
    void handle_update(StringHash eventType, VariantMap& eventData);
    /// Handle a background loaded resource completing.
    void HandleResourceBackgroundLoaded(StringHash eventType, VariantMap& eventData);
    /// Update asynchronous loading.
    void UpdateAsyncLoading();
    /// Finish asynchronous loading.
    void FinishAsyncLoading();
    /// Finish loading. Sets the scene filename and checksum.
    void FinishLoading(Deserializer* source);
    /// Finish saving. Sets the scene filename and checksum.
    void FinishSaving(Serializer* dest) const;
    /// Preload resources from a binary scene or object prefab file.
    void PreloadResources(File* file, bool isSceneFile);
    /// Preload resources from an XML scene or object prefab file.
    void PreloadResourcesXML(const XmlElement& element);
    /// Preload resources from a JSON scene or object prefab file.
    void PreloadResourcesJSON(const JSONValue& value);

    /// Replicated scene nodes by ID.
    HashMap<NodeId, Node*> replicatedNodes_;
    /// Local scene nodes by ID.
    HashMap<NodeId, Node*> localNodes_;
    /// Replicated components by ID.
    HashMap<ComponentId, Component*> replicatedComponents_;
    /// Local components by ID.
    HashMap<ComponentId, Component*> localComponents_;
    /// Cached tagged nodes by tag.
    HashMap<StringHash, Vector<Node*>> taggedNodes_;
    /// Asynchronous loading progress.
    AsyncProgress asyncProgress_;
    /// Node and component ID resolver for asynchronous loading.
    SceneResolver resolver_;
    /// Source file name.
    mutable String fileName_;
    /// Required package files for networking.
    Vector<SharedPtr<PackageFile>> requiredPackageFiles_;
    /// Registered node user variable reverse mappings.
    HashMap<StringHash, String> varNames_;
    /// Nodes to check for attribute changes on the next network update.
    HashSet<NodeId> networkUpdateNodes_;
    /// Components to check for attribute changes on the next network update.
    HashSet<ComponentId> networkUpdateComponents_;
    /// Delayed dirty notification queue for components.
    Vector<Component*> delayedDirtyComponents_;
    /// Mutex for the delayed dirty notification queue.
    std::mutex scene_mutex_;
    /// Preallocated event data map for smoothing update events.
    VariantMap smoothingData_;
    /// Next free non-local node ID.
    NodeId replicatedNodeID_;
    /// Next free non-local component ID.
    ComponentId replicatedComponentID_;
    /// Next free local node ID.
    NodeId localNodeID_;
    /// Next free local component ID.
    ComponentId localComponentID_;
    /// Scene source file checksum.
    mutable hash32 checksum_;
    /// Maximum milliseconds per frame to spend on async scene loading.
    int asyncLoadingMs_;
    /// Scene update time scale.
    float timeScale_;
    /// Elapsed time accumulator.
    float elapsedTime_;
    /// Motion smoothing constant.
    float smoothing_constant_;
    /// Motion smoothing snap threshold.
    float snap_threshold_;
    /// Update enabled flag.
    bool updateEnabled_;
    /// Asynchronous loading flag.
    bool asyncLoading_;
    /// Threaded update flag.
    bool threadedUpdate_;
};

using SlotSceneUpdate = Slot<Scene*, float>;
using SlotScenePostUpdate = Slot<Scene*, float>;

/// Register Scene library objects.
void DV_API register_scene_library();

} // namespace dviglo
