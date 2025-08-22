# 📝 Post-it Notes - Aplicación de Notas Adhesivas

Una aplicación moderna de notas Post-it desarrollada en C++11 usando la API nativa de Windows, con interfaz gráfica completa y funcionalidades avanzadas.

![C++](https://img.shields.io/badge/C++-11-blue.svg) ![Windows](https://img.shields.io/badge/Platform-Windows-green.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## 🚀 Características Principales

### 🎯 **Interfaz Principal**
- **Lista inteligente** con todos los Post-its y estado visual (oculto/visible)
- **Ventana principal** que inicia minimizada en la bandeja del sistema
- **Actualización en tiempo real** de títulos cuando se modifican
- **Botones de gestión**: Nuevo, Eliminar, Mostrar, Ocultar/Mostrar Todos

### 🔧 **Post-its Individuales**
- ✅ **Título y contenido editables** con guardado automático
- 🎨 **Selector de color** con paleta personalizable y contraste automático
- 🌟 **Control de transparencia** con slider interactivo  
- 📐 **Ventanas redimensionables y movibles**
- 💾 **Auto-guardado cada 5 segundos**
- 🎨 **Aplicación de color a todos los elementos** (fondo, campos de texto)

### 🖥️ **Sistema de Bandeja**
- 🔘 **Ícono personalizado** en la bandeja del sistema
- **Doble clic**: Mostrar ventana principal
- **Clic derecho**: Menú contextual completo
  - Mostrar Ventana Principal
  - Nuevo Post-it
  - Mostrar/Ocultar Todos los Post-its
  - Salir

### ⚡ **Optimizaciones Avanzadas**
- 🧠 **Gestión inteligente de memoria** con cache de recursos
- ⏱️ **Auto-guardado con timer** optimizado
- 🔄 **Recreación automática** de ventanas destruidas
- 📊 **Validación robusta** de datos e índices
- 🎯 **Actualizaciones eficientes** con delays inteligentes

## 📁 Estructura del Proyecto

```
post-it2/
├── 📂 src/
│   ├── 🚀 main.cpp           # Punto de entrada 
│   ├── 🏗️ PostIt.cpp         # Implementación principal
│   ├── 🎨 postit.ico         # Icono de la aplicación
│   └── 📄 postit.rc          # Archivo de recursos
├── 📂 include/
│   └── 📋 PostIt.h           # Definiciones de clases
├── 📂 build/                 # Archivos compilados
│   ├── ⚙️ *.o                # Objetos compilados
│   ├── 📦 postit.res         # Recursos compilados
│   └── 🎯 PostIt.exe         # Ejecutable final
├── 🔧 Makefile              # Sistema de compilación
└── 📖 README.md             # Documentación
```

## 🛠️ Compilación y Ejecución

### ✅ **Requisitos Previos**
- **MinGW-w64** o **MSYS2** con g++
- **Windows SDK** (para librerías del sistema)
- **Make** para el sistema de construcción

### 🔨 **Compilación Rápida**

```bash
# Compilar proyecto completo
mingw32-make

# Compilar desde cero
mingw32-make clean && mingw32-make

# Ejecutar aplicación
.\build\PostIt.exe
```

### 🎯 **Compilación Manual** (si necesario)

```bash
# Compilar recursos
windres src/postit.rc -O coff -o build/postit.res

# Compilar código fuente
g++ -DUNICODE -D_UNICODE -std=c++11 -Wall -Wextra -I include ^
    -c src/*.cpp -o build/

# Enlazar ejecutable
g++ build/*.o build/postit.res -o build/PostIt.exe ^
    -mwindows -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32
```

## 🎮 Uso de la Aplicación

### 🚀 **Inicio Rápido**
1. **Ejecuta** `PostIt.exe` - la aplicación se minimiza automáticamente a la bandeja
2. **Clic derecho** en el ícono de la bandeja → "Nuevo Post-it"  
3. **Edita** el título y contenido del Post-it
4. **Personaliza** color y transparencia según tus preferencias
5. **Todo se guarda automáticamente** cada 5 segundos

### 📋 **Gestión desde Ventana Principal**
- **Doble clic** en ícono de bandeja para abrir la ventana principal
- **Ver lista** de todos los Post-its con estado visual
- **Doble clic** en cualquier Post-it de la lista para mostrarlo
- **Usar botones** para gestionar Post-its:
  - 🆕 **Nuevo Post-it**: Crear nuevo
  - 🗑️ **Eliminar**: Borrar Post-it seleccionado (con confirmación)
  - 👁️ **Mostrar**: Mostrar Post-it seleccionado
  - 🙈 **Ocultar Todos**: Ocultar todos los Post-its
  - 👀 **Mostrar Todos**: Mostrar todos los Post-its

## 🔧 Características Técnicas

### 🏗️ **Arquitectura**
- **Patrón Singleton** para el gestor principal
- **Gestión automática de recursos** con destructores inteligentes  
- **Persistencia binaria** optimizada para velocidad
- **API nativa de Windows** para máximo rendimiento

### 💾 **Persistencia de Datos**
- **Archivo**: `postits.dat` (formato binario optimizado)
- **Auto-guardado**: Cada 5 segundos automáticamente
- **Guardado manual**: Al modificar título, texto, color o transparencia
- **Carga automática**: Al iniciar la aplicación

### ⚡ **Optimizaciones**
- **Cache de iconos y brushes** para evitar recargas
- **Timer inteligente** para actualización de listas (500ms delay)
- **Validación robusta** de índices y ventanas
- **Recreación automática** de ventanas destruidas
- **Límite configurable** de Post-its (máximo 100)

## 🎨 Personalización

### 🌈 **Colores**
- **Selector estándar de Windows** con colores personalizados predefinidos
- **Contraste automático** - texto negro/blanco según luminancia del fondo
- **Aplicación completa** - color se aplica a fondo, campos de texto y etiquetas

### 🌟 **Transparencia**
- **Rango**: 50-255 (de más transparente a completamente opaco)
- **Control en tiempo real** con slider interactivo
- **Persistencia** - se mantiene entre sesiones

## 📊 Especificaciones

| Característica | Detalles |
|---|---|
| **Lenguaje** | C++11 con API nativa de Windows |
| **Compilador** | MinGW-w64, MSVC compatible |
| **Dependencias** | user32.lib, gdi32.lib, comctl32.lib, comdlg32.lib, shell32.lib |
| **Plataforma** | Windows 7/8/10/11 (32/64-bit) |
| **Tamaño ejecutable** | ~150KB (con recursos incluidos) |
| **Memoria** | <5MB en tiempo de ejecución |
| **Persistencia** | Archivo binario (`postits.dat`) |

## 🐛 Resolución de Problemas

### ❌ **Problemas Comunes**

**El ejecutable no inicia:**
- Verifica que tienes todas las librerías de Windows necesarias
- Ejecuta desde terminal para ver mensajes de error

**Los Post-its no se guardan:**
- Verifica permisos de escritura en el directorio
- Comprueba que el archivo `postits.dat` no esté en solo lectura

**El ícono no aparece en la bandeja:**
- Reinicia la aplicación
- Verifica que la bandeja del sistema esté habilitada

## 🤝 Contribuciones

¡Las contribuciones son bienvenidas! Por favor:

1. **Fork** el repositorio
2. **Crea** una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. **Commit** tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. **Push** a la rama (`git push origin feature/AmazingFeature`)
5. **Abre** un Pull Request

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Ver archivo `LICENSE` para detalles.

## 👨‍💻 Autor

**Iván Otalvaro** - [@ivanotalvaro](https://github.com/ivanotalvaro)
0
---

⭐ **¡Dale una estrella si te gusta el proyecto!** ⭐

## Uso

1. Ejecutar `PostIt.exe`
2. La aplicación inicia **minimizada en la bandeja del sistema**
3. **Ícono en bandeja**:
   - **Doble clic**: Mostrar ventana principal
   - **Clic derecho**: Menú contextual con opciones:
     - "Mostrar Ventana Principal"
     - "Nuevo Post-it"
     - "Salir"
4. **Ventana principal**:
   - "Nuevo Post-it": Crear nuevas notas
   - **Doble clic** en lista: Mostrar Post-it seleccionado
   - Botón "Mostrar": Mostrar Post-it seleccionado
   - Botón "Eliminar": Borrar Post-it seleccionado
5. **Post-its individuales**:
   - Botón "Color": Cambiar color de fondo
   - Slider "Opacidad": Ajustar transparencia
   - **Guardado automático** de todos los cambios

## Dependencias

- Windows API (user32.lib, gdi32.lib)
- Common Controls (comctl32.lib)
- Common Dialog (comdlg32.lib)
- Shell API (shell32.lib) - Para ícono en bandeja del sistema

## Funcionalidades implementadas

- ✅ Ventana principal con lista de Post-its
- ✅ **Inicio minimizado en bandeja del sistema**
- ✅ **Ícono en system tray con menú contextual**
- ✅ **Mostrar Post-it al hacer doble clic en la lista**
- ✅ Creación de Post-its individuales
- ✅ Edición de título y texto
- ✅ Selector de color personalizable
- ✅ Control de transparencia funcional
- ✅ **Guardado automático inmediato**
- ✅ Persistencia de datos
- ✅ Ventanas redimensionables y movibles
