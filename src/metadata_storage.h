#ifndef HERMES_METADATA_STORAGE_H_
#define HERMES_METADATA_STORAGE_H_

namespace hermes {

/**
 *
 */
void PutToStorage(MetadataManager *mdm, const char *key, u64 val,
                MapType map_type);

/**
 *
 */
u64 GetFromStorage(MetadataManager *mdm, const char *key, MapType map_type);

/**
 *
 */
char *ReverseGetFromStorage(MetadataManager *mdm, u64 id, MapType map_type);
/**
 *
 */
void DeleteFromStorage(MetadataManager *mdm, const char *key, MapType map_type);

/**
 *
 */
u32 HashStringForStorage(MetadataManager *mdm, RpcContext *rpc,
                         const char *str);

/**
 *
 */
void SeedHashForStorage(size_t seed);

/**
 *
 */
size_t GetStoredMapSize(MetadataManager *mdm, MapType map_type);

/**
 *
 */
std::vector<u64> GetNodeTargets(SharedMemoryContext *context);

}  // namespace hermes

#endif  // HERMES_METADATA_STORAGE_H_
