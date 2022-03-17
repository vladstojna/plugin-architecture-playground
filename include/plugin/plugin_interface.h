#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

#pragma region macros

#define PASMP_VERSION_MAJOR 0
#define PASMP_VERSION_MINOR 1
#define PASMP_VERSION ((PASMP_VERSION_MAJOR << 16) | PASMP_VERSION_MINOR)

#define PASMP_CALL __cdecl
#define PASMP_CALLBACK PASMP_CALL
#define PASMP_API __declspec(dllexport)

#define PASMP_FUNCTION PASMP_API PASMP_status_t PASMP_CALL

#pragma endregion macros

#pragma region enumerations

typedef enum PASMP_status_e {
  PASMP_SUCCESS,
  PASMP_UNKNOWN,
  PASMP_NOT_IMPLEMENTED,
  PASMP_UNAVAILABLE,
  PASMP_INVALID_STATUS,
  PASMP_INVALID_ARGUMENT,
  PASMP_ERROR_ALLOC,
  PASMP_ERROR_FATAL,
  PASMP_ERROR_SYSTEM,
  PASMP_ERROR_TRUNCATED,
  PASMP_ERROR_OTHER,
  PASMP_ERROR_PERSISTENCE_PATH,
  PASMP_ERROR_PLUGIN,
  PASMP_ERROR_ACTION_NOENT,
  PASMP_ERROR_ACTION_EXEC,
  PASMP_ERROR_ACTION_OTHER,
  PASMP_ERROR_ACTION_SERIAL,
  PASMP_ERROR_PAYLOAD_INVALID,
} PASMP_status_t;

typedef enum PASMP_payload_tag_e {
  PASMP_PAYLOAD_NONE,
  PASMP_PAYLOAD_INT32,
  PASMP_PAYLOAD_INT64,
  PASMP_PAYLOAD_UINT32,
  PASMP_PAYLOAD_UINT64,
  PASMP_PAYLOAD_FLOAT,
  PASMP_PAYLOAD_DOUBLE,
  PASMP_PAYLOAD_STRING,
} PASMP_payload_tag_t;

typedef enum PASMP_config_status_e {
  PASMP_CONFIG_SUCCESS,
  PASMP_CONFIG_CANCEL,
} PASMP_config_status_t;

#pragma endregion

#pragma region types

typedef struct PASMP_error_descriptor_st {
  char *what = NULL;
  uint64_t size = 0;
} PASMP_error_descriptor_t;

typedef struct PASMP_string_view_st {
  const char *data;
  uint64_t size;
} PASMP_string_view_t;

typedef struct PASMP_version_st *PASMP_version_t;
typedef struct PASMP_plugin_descriptor_st *PASMP_plugin_descriptor_t;
typedef struct PASMP_action_descriptor_st *PASMP_action_descriptor_t;
typedef struct PASMP_plugin_attr_st *PASMP_plugin_attr_t;
typedef struct PASMP_plugin_st *PASMP_plugin_t;
typedef struct PASMP_action_collection_st *PASMP_action_collection_t;
typedef struct PASMP_action_st *PASMP_action_t;

typedef void(PASMP_CALLBACK PASMP_on_action_modified_t)(PASMP_action_t, void *);

typedef void(PASMP_CALLBACK PASMP_on_action_added_t)(PASMP_action_t, void *);

typedef void(PASMP_CALLBACK PASMP_on_action_removed_t)(PASMP_action_t, void *);

typedef void(PASMP_CALLBACK PASMP_on_config_finish_t)(PASMP_status_t,
                                                      PASMP_config_status_t,
                                                      void *);

typedef void(PASMP_CALLBACK PASMP_on_action_finish_t)(PASMP_status_t,
                                                      PASMP_string_view_t,
                                                      void *);

typedef union PASMP_payload_data_st {
  int32_t int32_value;
  int64_t int64_value;
  uint32_t uint32_value;
  uint64_t uint64_value;
  float float_value;
  double double_value;
  PASMP_string_view_t string_value;
} PASMP_payload_data_t;

typedef struct PASMP_payload_st {
  PASMP_payload_tag_t tag = PASMP_PAYLOAD_NONE;
  const PASMP_payload_data_t *data = NULL;
} PASMP_payload_t;

#pragma endregion

#pragma region misc_operations

PASMP_API uint32_t PASMP_CALL PASMP_version();
PASMP_API int32_t PASMP_CALL PASMP_is_library_compatible(uint32_t);

#pragma endregion

#pragma region version_operations

PASMP_FUNCTION PASMP_version_create(PASMP_version_t *,
                                    PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_version_destroy(PASMP_version_t);

PASMP_API uint16_t PASMP_CALL PASMP_version_major(PASMP_version_t);
PASMP_API uint16_t PASMP_CALL PASMP_version_minor(PASMP_version_t);
PASMP_API uint16_t PASMP_CALL PASMP_version_patch(PASMP_version_t);
PASMP_API PASMP_string_view_t PASMP_CALL PASMP_version_pre(PASMP_version_t);
PASMP_API PASMP_string_view_t PASMP_CALL PASMP_version_build(PASMP_version_t);

#pragma endregion

#pragma region plugin_operations

PASMP_FUNCTION PASMP_plugin_attr_create(PASMP_plugin_attr_t *,
                                        PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_plugin_attr_destroy(PASMP_plugin_attr_t);
PASMP_FUNCTION PASMP_plugin_attr_on_action_mod(PASMP_plugin_attr_t,
                                               PASMP_on_action_modified_t *,
                                               void *,
                                               PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_plugin_attr_on_action_add(PASMP_plugin_attr_t,
                                               PASMP_on_action_added_t *,
                                               void *,
                                               PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_plugin_attr_on_action_rm(PASMP_plugin_attr_t,
                                              PASMP_on_action_removed_t *,
                                              void *,
                                              PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_plugin_attr_persistence_path(PASMP_plugin_attr_t,
                                                  PASMP_string_view_t,
                                                  PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_plugin_descriptor_create(PASMP_plugin_descriptor_t *,
                                              PASMP_error_descriptor_t *);
PASMP_FUNCTION
PASMP_plugin_descriptor_destroy(PASMP_plugin_descriptor_t);

PASMP_API PASMP_string_view_t PASMP_CALL
PASMP_plugin_name(PASMP_plugin_descriptor_t, int32_t short_variant);

PASMP_API PASMP_string_view_t PASMP_CALL
PASMP_plugin_description(PASMP_plugin_descriptor_t, int32_t short_variant);

PASMP_FUNCTION PASMP_plugin_create(PASMP_plugin_t *, PASMP_plugin_attr_t,
                                   PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_plugin_addref(PASMP_plugin_t);
PASMP_FUNCTION PASMP_plugin_release(PASMP_plugin_t);

PASMP_FUNCTION PASMP_action_collection_create(PASMP_action_collection_t *,
                                              PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_action_collection_destroy(PASMP_action_collection_t);
PASMP_API
uint64_t PASMP_CALL PASMP_action_collection_size(PASMP_action_collection_t);
PASMP_FUNCTION PASMP_action_collection_at(PASMP_action_collection_t, uint64_t,
                                          PASMP_action_t *,
                                          PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_plugin_actions(PASMP_plugin_t, PASMP_action_collection_t,
                                    PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_plugin_configure_gui(PASMP_plugin_t,
                                          PASMP_on_config_finish_t *, void *,
                                          PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_plugin_configure_cli(PASMP_plugin_t,
                                          PASMP_on_config_finish_t *, void *,
                                          PASMP_error_descriptor_t *);

#pragma endregion

#pragma region action_operations

PASMP_FUNCTION PASMP_action_serialize(PASMP_action_t, char *, uint64_t *,
                                      PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_action_deserialize(PASMP_action_t *, const char *,
                                        uint64_t, PASMP_error_descriptor_t *);
PASMP_FUNCTION PASMP_action_destroy(PASMP_action_t);

PASMP_FUNCTION PASMP_action_descriptor_create(PASMP_plugin_t, PASMP_action_t,
                                              PASMP_action_descriptor_t *,
                                              PASMP_error_descriptor_t *);
PASMP_FUNCTION
PASMP_action_descriptor_destroy(PASMP_action_descriptor_t);

PASMP_API PASMP_string_view_t PASMP_CALL
PASMP_action_name(PASMP_action_descriptor_t, int32_t short_variant);

PASMP_API PASMP_string_view_t PASMP_CALL
PASMP_action_description(PASMP_action_descriptor_t, int32_t short_variant);

PASMP_FUNCTION
PASMP_action_execute(PASMP_plugin_t, PASMP_action_t, PASMP_payload_t,
                     PASMP_error_descriptor_t *);

PASMP_FUNCTION PASMP_action_execute_async(PASMP_plugin_t, PASMP_action_t,
                                          PASMP_payload_t,
                                          PASMP_on_action_finish_t *, void *,
                                          PASMP_error_descriptor_t *);

PASMP_API uint64_t PASMP_CALL PASMP_action_hash(PASMP_action_t);
PASMP_API int32_t PASMP_CALL PASMP_action_equal(PASMP_action_t, PASMP_action_t);

#pragma endregion

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)
