#define _CRT_SECURE_NO_WARNINGS

#include "components.h"

#include <cglm/cglm.h>
#include <cJSON.h>
#include <string.h>
#include <imgui_impl.h>

#include "utils.h"
#include "containers/hashtable.h"
#include "component_defs.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

static Entity *s_inspector_entity_to_assign;

bool components_inspector_wants_entity( void )
{
    return s_inspector_entity_to_assign != NULL;
}

void components_inspector_provide_entity( Entity e )
{
    if( !s_inspector_entity_to_assign ) return;
    *s_inspector_entity_to_assign = e;
    s_inspector_entity_to_assign = NULL;
}

static const ComponentInfo *get_info_for_component_type( const char *type_name )
{
    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];
        if( strcmp( info->name, type_name ) == 0 ) return info;
    }

    PANIC("Unrecognized component type in get_info_for_component_type");
}

static void inspect_int   ( const char *label, int    *v ) { igInputInt(label, v, 1, 10, 0); }
static void inspect_float ( const char *label, float  *v ) { igDragFloat(label, v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_bool  ( const char *label, bool   *v ) { igCheckbox(label, v); }
static void inspect_vec2  ( const char *label, vec2   *v ) { igDragFloat2(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_vec3  ( const char *label, vec3   *v ) { igDragFloat3(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_vec4  ( const char *label, vec4   *v ) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_versor( const char *label, versor *v ) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); glm_quat_normalize(*v); }
static void inspect_mat4  ( const char *label, mat4   *v ) { igText("%s {Matrix}", label); }
static void inspect_Vec_T ( const char *label, void   *v ) { igText("%s {Vec<T>}", label); }

static void inspect_Entity( ECS *ecs, const char *label, Entity *v ) 
{
    bool name_from_transform;
    const char *entity_name = components_name_entity( ecs, *v, &name_from_transform );

    char name_buffer[256];
    snprintf( name_buffer, 256, name_from_transform ? "%s" : "(%s)", entity_name );

    if( igButton( name_buffer, (ImVec2){ 0, 0 } ) )
        s_inspector_entity_to_assign = v;

    igSameLine( 0, -1 );
    igText( "%s", label );
}

static void inspect_string( const char *label, char **v )
{
    if( !*v )
    {
        *v = malloc(1); 
        (*v)[0] = 0;
    }

    char buf[1024] = "";
    buf[1023] = 0;
    strncpy( buf, *v, 1023 );
    igInputText( label, buf, 1024, 0, NULL, NULL );

    if( strcmp( buf, *v ) != 0 )
    {
        free( *v );
        size_t len = strlen( buf );
        *v = malloc( len + 1 );
        strncpy( *v, buf, len );
        (*v)[len] = 0;
    }
}

static void inspect_component( ECS *ecs, void *component, const char *label, const ComponentInfo *info );

static void inspect_field( ECS *ecs, void *field, const ComponentField *field_def )
{
    if( field_def->flags & COMPONENT_FLAG_IS_VEC )
    {
        inspect_Vec_T( field_def->name, NULL );
        return;
    }

    switch( field_def->type )
    {
    case COMPONENT_FIELD_TYPE_INT:     inspect_int( field_def->name, field );    return;
    case COMPONENT_FIELD_TYPE_FLOAT:   inspect_float( field_def->name, field );  return;
    case COMPONENT_FIELD_TYPE_BOOL:    inspect_bool( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC2:    inspect_vec2( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC3:    inspect_vec3( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC4:    inspect_vec4( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VERSOR:  inspect_versor( field_def->name, field ); return;
    case COMPONENT_FIELD_TYPE_MAT4:    inspect_mat4( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_STRING:  inspect_string( field_def->name, field ); return;
    case COMPONENT_FIELD_TYPE_POINTER: return;
    case COMPONENT_FIELD_TYPE_ENTITY:  inspect_Entity( ecs, field_def->name, field ); return;

    case COMPONENT_FIELD_TYPE_SUBCOMPONENT:
        inspect_component( ecs, field, field_def->name, get_info_for_component_type( field_def->subcomponent_name ) );
        return;
    }

    PANIC("Unhandled field type in inspect_field");
}

static void inspect_component( ECS *ecs, void *component, const char *label, const ComponentInfo *info )
{
    if (label) 
    { 
        igSeparator(); 
        igText( label ); 
        igSeparator(); 
    }

    igPushIDPtr( component );

    for( int i = 0; i < info->num_fields; ++ i )
    {
        const ComponentField *field = &info->fields[i];

        if( ( field->flags & COMPONENT_FLAG_HIDDEN ) == 0 ) 
        {
            igPushIDInt( i );
            inspect_field( ecs, (uint8_t*)component + field->offset, field );
            igPopID();
        }
    }

    igPopID();
}

ECS *components_ecs_new( void )
{
    ECS *ecs = ecs_new();

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];
        ecs_register_component( ecs, info->name, info->size, info->destructor );
    }

    return ecs;
}

const char *components_name_entity( const ECS *ecs, Entity e, bool *name_from_transform )
{
    if( !e ) return "empty";

    ECS_VIEW_COMPONENT_DECL( Transform, t, ecs, e );
    if( t && t->name && strlen( t->name ) ) 
    {
        *name_from_transform = true;
        return t->name;
    }

    *name_from_transform = false;

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
        if( ecs_view_component( ecs, e, COMPONENTS_ALL_INFOS[i]->name ) )
            return COMPONENTS_ALL_INFOS[i]->name;

    return "empty";
}

void components_generic_destruct( const ComponentInfo *info, void *component )
{
    for( int i = 0; i < info->num_fields; ++i )
    {
        const ComponentField *field = &info->fields[i];
        void *field_ptr = (uint8_t*)component + field->offset;

        if( field->flags & COMPONENT_FLAG_IS_VEC )
        {
            Vec *vec = (Vec*)field_ptr;

            if( field->type == COMPONENT_FIELD_TYPE_SUBCOMPONENT )
            {
                for( int i = 0; i < vec->item_count; ++i )
                    components_generic_destruct( get_info_for_component_type( field->subcomponent_name ), vec_at( vec, i ) );
            }
            else if( field->type == COMPONENT_FIELD_TYPE_STRING )
            {
                for( int i = 0; i < vec->item_count; ++i )
                    free( *(char**)vec_at( vec, i ) );
            }

            vec_clear( vec );
        }
        else if( field->type == COMPONENT_FIELD_TYPE_SUBCOMPONENT )
        {
            components_generic_destruct( get_info_for_component_type( field->subcomponent_name ), field_ptr );
        }
        else if( field->type == COMPONENT_FIELD_TYPE_STRING )
        {
            char **string = (char**)field_ptr;
            free( *string );
            *string = NULL;
        }
    }
}

static void *add_component_if_missing( ECS *ecs, Entity e, const char *type_name )
{
    void *comp =  ecs_borrow_component( ecs, e, type_name, __FILE__, __LINE__ );

    if( !comp )
    {
        comp = ecs_add_component_zeroed( ecs, e, type_name, __FILE__, __LINE__ );
        const ComponentInfo *comp_info = get_info_for_component_type( type_name );
        memcpy( comp, comp_info->prototype, comp_info->size );
    }

    return comp;
}

void components_inspect_entity( ECS *ecs, Entity e )
{
    int visible_component_count = 0;
    const char *visible_components[COMPONENTS_TOTAL_COUNT];

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        if( COMPONENTS_ALL_INFOS[i]->flags & COMPONENT_FLAG_HIDDEN ) continue;

        visible_components[visible_component_count] = COMPONENTS_ALL_INFOS[i]->name;
        visible_component_count++;
    }

    static int selected_component = 0;

    igCombo( "", &selected_component, visible_components, visible_component_count, 0 );
    igSameLine( 0, -1 );

    if( igButton( "Add Component", (ImVec2){ 0, 0 } ) )
        add_component_if_missing( ecs, e, visible_components[selected_component] );

    igSeparator();

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];

        bool keep_alive = true;
        void *component = ecs_borrow_component( ecs, e, info->name, __FILE__, __LINE__ );

        if( component && igCollapsingHeaderBoolPtr( info->name, &keep_alive, ImGuiTreeNodeFlags_DefaultOpen ) )
            inspect_component( ecs, component, NULL, info );

        if (! keep_alive)
            ecs_remove_component( ecs, e, info->name );
    }
}

static void serialize_component( cJSON *obj, const void *component, const ComponentInfo *info, bool nested );

static cJSON *serialize_field( void *field, const ComponentField *field_def )
{
    switch( field_def->type )
    {
    case COMPONENT_FIELD_TYPE_INT:    return cJSON_CreateNumber(*(int*)field);
    case COMPONENT_FIELD_TYPE_FLOAT:  return cJSON_CreateNumber(*(float*)field);
    case COMPONENT_FIELD_TYPE_BOOL:   return cJSON_CreateBool  (*(bool*)field); 
    case COMPONENT_FIELD_TYPE_VEC2:   return cJSON_CreateFloatArray((float*)(*(vec2*)field), 2);
    case COMPONENT_FIELD_TYPE_VEC3:   return cJSON_CreateFloatArray((float*)(*(vec3*)field), 3);
    case COMPONENT_FIELD_TYPE_VEC4:   return cJSON_CreateFloatArray((float*)(*(vec4*)field), 4);
    case COMPONENT_FIELD_TYPE_VERSOR: return cJSON_CreateFloatArray((float*)(*(versor*)field), 4);
    case COMPONENT_FIELD_TYPE_MAT4:   return cJSON_CreateFloatArray((float*)(*(mat4*)field), 16);
    case COMPONENT_FIELD_TYPE_ENTITY: return cJSON_CreateNumber((double)(*(Entity*)field));
    case COMPONENT_FIELD_TYPE_STRING: return cJSON_CreateString(*(char**)field);

    case COMPONENT_FIELD_TYPE_SUBCOMPONENT: {}
        cJSON *obj = cJSON_CreateObject();
        serialize_component( obj, field, get_info_for_component_type( field_def->subcomponent_name ), true );
        return obj;
    }

    PANIC("Unhandled field type in serialize_field");
}

static void serialize_component( cJSON *obj, const void *component, const ComponentInfo *info, bool nested )
{
    cJSON *comp_obj = nested ? obj : cJSON_AddObjectToObject( obj, info->name );

    for( int i = 0; i < info->num_fields; ++i )
    {
        const ComponentField *field = &info->fields[i];
        void *field_ptr = (uint8_t*)component + field->offset;

        if( field->flags & COMPONENT_FLAG_DONT_SERIALIZE || field->type == COMPONENT_FIELD_TYPE_POINTER ) continue;

        if( field->flags & COMPONENT_FLAG_IS_VEC )
        {
            // TODO implement
        }
        else
        {
            cJSON_AddItemToObject( comp_obj, field->name, serialize_field( field_ptr, field ) );
        }
    }
}

static void deserialize_nested_component( cJSON *item, void *out, const ComponentInfo *info, HashTable *entities_for_ids );

static void deserialize_field( cJSON *item, void *out, HashTable *entities_for_ids, const ComponentField *field_def )
{
    switch( field_def->type )
    {
    case COMPONENT_FIELD_TYPE_INT:    *(int*)  out = (int)item->valuedouble;   return;
    case COMPONENT_FIELD_TYPE_FLOAT:  *(float*)out = (float)item->valuedouble; return;
    case COMPONENT_FIELD_TYPE_BOOL:   *(bool*) out = cJSON_IsTrue(item);       return;
    case COMPONENT_FIELD_TYPE_VEC2:   for( int i = 0; i < 2;  ++i ) (*(vec2*)  out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; return;
    case COMPONENT_FIELD_TYPE_VEC3:   for( int i = 0; i < 3;  ++i ) (*(vec3*)  out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; return;
    case COMPONENT_FIELD_TYPE_VEC4:   for( int i = 0; i < 4;  ++i ) (*(vec4*)  out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; return;
    case COMPONENT_FIELD_TYPE_VERSOR: for( int i = 0; i < 4;  ++i ) (*(versor*)out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; return;
    case COMPONENT_FIELD_TYPE_MAT4:   for( int i = 0; i < 16; ++i ) (*(vec2*)  out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; return;
    case COMPONENT_FIELD_TYPE_STRING: *(char**)out = strdup(cJSON_GetStringValue(item)); return;

    case COMPONENT_FIELD_TYPE_ENTITY: {}
        char id_as_str_buf[128];
        sprintf( id_as_str_buf, "%f", item->valuedouble );
        *(Entity*)out = *(Entity*)hashtable_at( entities_for_ids, id_as_str_buf );
        return;

    case COMPONENT_FIELD_TYPE_SUBCOMPONENT:
        deserialize_nested_component( 
            cJSON_GetObjectItem( item, field_def->name ), 
            out, 
            get_info_for_component_type( field_def->subcomponent_name ),
            entities_for_ids
        );
        return;
    }

    PANIC("Unhandled field type in deserialize_field");
}

static void deserialize_nested_component( cJSON *item, void *out, const ComponentInfo *info, HashTable *entities_for_ids )
{
    const ComponentField *field;
    for( int i = 0; i < info->num_fields; ++i )
    {
        field = &info->fields[i];
        void *field_ptr = (uint8_t*)out + field->offset;

        if( field->flags & COMPONENT_FLAG_DONT_SERIALIZE || field->type == COMPONENT_FIELD_TYPE_POINTER ) continue;

        if( field->flags & COMPONENT_FLAG_IS_VEC )
        {
            // TODO implement
        }
        else
        {
            deserialize_field( cJSON_GetObjectItem( item, field->name ), field_ptr, entities_for_ids, field );
        }
    }
}

static void deserialize_component( ECS *ecs, cJSON *component_obj, Entity entity, const ComponentInfo *info, HashTable *entities_for_ids )
{
    void *comp = add_component_if_missing( ecs, entity, info->name );
    deserialize_nested_component( component_obj, comp, info, entities_for_ids );
    ecs_return_component( ecs, comp, __FILE__, __LINE__ );
}

char *components_serialize_scene_alloc( const ECS *ecs )
{
    cJSON *json = cJSON_CreateArray();
    size_t num_entities;
    Entity *entities = ecs_find_all_entities_alloc( ecs, &num_entities );

    for( int i = 0; i < num_entities; ++i ) 
    {
        bool empty = true;
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject( obj, "_id", (double)entities[i] );
    
        for( int j = 0; j < COMPONENTS_TOTAL_COUNT; ++j )
        {
            if( COMPONENTS_ALL_INFOS[j]->flags & COMPONENT_FLAG_DONT_SERIALIZE ) continue;

            const void *component = ecs_view_component( ecs, entities[i], COMPONENTS_ALL_INFOS[j]->name );
            if( component )
            {
                serialize_component( obj, component, COMPONENTS_ALL_INFOS[j], false );
                empty = false;
            }
        }

        if( empty )
            cJSON_Delete( obj );
        else
            cJSON_AddItemToArray( json, obj );
    }

    char *result = cJSON_Print( json );
    free( entities );
    cJSON_Delete( json );
    return result;
}

ECS *components_deserialize_scene_alloc( const char *json_scene )
{
    ECS *result = components_ecs_new();
    cJSON *json = cJSON_Parse( json_scene );
    HashTable entities_for_ids = hashtable_empty( 1024, sizeof( Entity ) );
    char id_as_str_buf[128];
    cJSON *entity_obj;

    Entity empty_entity = 0;
    sprintf( id_as_str_buf, "%f", 0.0 );
    hashtable_set_copy( &entities_for_ids, id_as_str_buf, &empty_entity );

    cJSON_ArrayForEach( entity_obj, json )
    {
        sprintf( id_as_str_buf, "%f", cJSON_GetObjectItem( entity_obj, "_id" )->valuedouble );
        Entity new_entity = ecs_create_entity( result );
        hashtable_set_copy( &entities_for_ids, id_as_str_buf, &new_entity );
    }

    cJSON_ArrayForEach( entity_obj, json )
    {
        sprintf( id_as_str_buf, "%f", cJSON_GetObjectItem( entity_obj, "_id" )->valuedouble );
        Entity e = *(Entity*)hashtable_at( &entities_for_ids, id_as_str_buf );

        cJSON *component_obj;
        cJSON_ArrayForEach( component_obj, entity_obj )
        {
            char *type_name = component_obj->string;
            if( strcmp( "_id", type_name ) == 0 ) continue;

            for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i ) 
                if( strcmp( type_name, COMPONENTS_ALL_INFOS[i]->name ) == 0 )
                    deserialize_component( result, component_obj, e, COMPONENTS_ALL_INFOS[i], &entities_for_ids );
        }
    }
    
    hashtable_clear( &entities_for_ids );
    cJSON_Delete( json );
    return result;
}
