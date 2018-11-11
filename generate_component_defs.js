const components = require('./components.json');


const writeStructDef = type =>
{
    const result = [`typedef struct ${type.name}`, '{'];

    const fieldDefs = type.fields.map(field => field.vec 
        ? `    Vec ${field.name}; // of ${field.type}` 
        : field.type === 'string' 
            ? `    char *${field.name};` 
            : `    ${field.type} ${field.name};`);

    [].push.apply(result, fieldDefs);
    [].push.apply(result, ['}', `${type.name};`]);

    return result.join('\n');
};


const writeDefaultDef = (types, rootType) =>
{
    const writeDefaultForTypeName = typeName => {
        switch (typeName) {
            case 'int': return '0';
            case 'float': return '0.f';
            case 'bool': return '0';
            case 'vec2': return '{0.f,0.f}';
            case 'vec3': return '{0.f,0.f,0.f}';
            case 'vec4': return '{0.f,0.f,0.f,0.f}';
            case 'versor': return '{0.f,0.f,0.f,1.f}';
            case 'mat4': return '{{1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},{0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}';
            case 'Entity': return '0';
            case 'string': return '0';
        }

        for (let i = 0; i < types.length; ++i)
            if (typeName === types[i].name)
                return writeDefaultForType(types[i]);

        return '#';
    };

    const writeDefaultForType = type =>
        '{' + type.fields
            .map(field => writeDefaultForField(field))
            .join(',') + '}';

    const writeDefaultForField = field => {
        if (field.default) return field.default;

        if (field.vec) {
            const typeName = field.type === 'string' ? 'char*' : field.type;
            return `{sizeof(${typeName}),0,0}`;
        }

        return writeDefaultForTypeName(field.type);
    };

    return `static const ${rootType.name} ${rootType.name}_default = ${writeDefaultForTypeName(rootType.name)};`;
};


const writeTypeInfo = (types, rootType) =>
{
    const writeFieldType = typeName => {
        switch (typeName) {
            case 'int': return 'COMPONENT_FIELD_TYPE_INT';
            case 'float': return 'COMPONENT_FIELD_TYPE_FLOAT';
            case 'bool': return 'COMPONENT_FIELD_TYPE_BOOL';
            case 'vec2': return 'COMPONENT_FIELD_TYPE_VEC2';
            case 'vec3': return 'COMPONENT_FIELD_TYPE_VEC3';
            case 'vec4': return 'COMPONENT_FIELD_TYPE_VEC4';
            case 'versor': return 'COMPONENT_FIELD_TYPE_VERSOR';
            case 'mat4': return 'COMPONENT_FIELD_TYPE_MAT4';
            case 'Entity': return 'COMPONENT_FIELD_TYPE_ENTITY';
            case 'string': return 'COMPONENT_FIELD_TYPE_STRING';
            default: return 'COMPONENT_FIELD_TYPE_SUBCOMPONENT';
        }
    };

    const writeFlags = item => {
        let flags = '0';
        if (item.hide === true) flags += ' | COMPONENT_FLAG_HIDDEN';
        if (item.vec === true) flags += ' | COMPONENT_FLAG_IS_VEC';
        if (item.serialize === false) flags += ' | COMPONENT_FLAG_DONT_SERIALIZE';
        return flags;
    };

    const writeSubName = field =>
        writeFieldType(field.type) === 'COMPONENT_FIELD_TYPE_SUBCOMPONENT'
            ? `"${field.type}"`
            : 'NULL';

    const writeFieldInfo = field => 
        `    { "${field.name}", ${writeFieldType(field.type)}, ${writeFlags(field)}, ${writeSubName(field)} }`;

    const fieldInfos = rootType.fields.map(writeFieldInfo).join(',\n');

    return `static const ComponentInfo ${rootType.name}_info = { "${rootType.name}", ` +
        `&${rootType.name}_destruct, ${writeFlags(rootType)}, ${rootType.fields.length}, {\n${fieldInfos}\n}};`;
};


const writeDestructor = (type, body) => {
    let result = `static inline void ${type.name}_destruct( ${type.name} *x )`;
    if (body)
        result += `{ components_generic_destruct( &${type.name}_info, x ); }`;
    else
        result += ';';
    return result;
};


const writeAllInfosArray = types => {
    const infoPtrs = types.map(t => `&${t.name}_info`).join(', ');

    return `static const size_t COMPONENTS_ALL_INFOS_COUNT = ${types.length};\n` +
           `static const ComponentInfo *COMPONENTS_ALL_INFOS[] = { ${infoPtrs} };`;
};


console.log('#pragma once');
console.log('#include <cglm/cglm.h>');
console.log('#include <stdint.h>');
console.log('#include <stddef.h>');
console.log('#include "containers/vec.h"');
console.log('#include "components.h"');
console.log('');

components.forEach(c => {
    console.log(writeStructDef(c));
    console.log('');
});

components.forEach(c => {
    console.log(writeDestructor(c, false));
    console.log(writeDefaultDef(components, c));
    console.log(writeTypeInfo(components, c));
    console.log(writeDestructor(c, true));
    console.log('');
});


console.log(writeAllInfosArray(components));
