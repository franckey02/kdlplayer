# kdlplayer

# 🎵 kdlplayer
**kdlplayer** es un reproductor de música minimalista y eficiente desarrollado en **C++** utilizando la biblioteca **raylib**. Está diseñado específicamente para entornos Linux, ofreciendo integración nativa con el escritorio y soporte para **Discord Rich Presence**.

---

## ✨ Características Principaless

* **⚡ Ligero y Rápido:** Construido sobre raylib para un consumo mínimo de recursos.
* **🎮 Discord Rich Presence:** Muestra automáticamente qué canción estás escuchando y tu progreso en tu perfil de Discord.
* **📂 Integración con el Sistema:** * Diálogo nativo para seleccionar carpetas (vía `zenity`).
* Generación automática de archivo `.desktop` e icono para el lanzador de aplicaciones.


* **🎹 Controles Intuitivos:** Soporte para teclado, ratón y rueda de desplazamiento.
* **🎶 Gestión de Playlist:** Soporte para formatos `.mp3`, `.flac` y `.ogg` con modos de reproducción aleatoria (Shuffle) y bucle.
* **🎨 Interfaz Personalizable:** Soporte para fuentes externas (`font.ttf`) y renderizado con suavizado (MSAA 4X).

---

## 🛠️ Requisitos del Sistema

Para compilar y ejecutar este proyecto, necesitarás:

* **Compilador:** GCC/G++ con soporte para C++17.
* **Bibliotecas:**
* [raylib](https://www.raylib.com/)
* [Discord-RPC](https://github.com/discord/discord-rpc)


* **Utilidades de Linux:** `zenity` (para el explorador de archivos) y `update-desktop-database`.

---

## 🚀 Instalación y Compilación

1. **Clona el repositorio:**
```bash
git clone https://github.com/tu-usuario/kdlplayer.git
cd kdlplayer

```


2. **Asegúrate de tener las dependencias instaladas:**
```bash
# Ejemplo en Ubuntu/Debian
sudo apt install libraylib-dev zenity

```


3. **Compilación:**
Asegúrate de enlazar las librerías correspondientes en tu comando de compilación:
```bash
g++ mixerplayer.cpp -o kdlplayer -lraylib -ldiscord-rpc -lpthread -lrt -lm -ldl

```


4. **Recursos:**
Coloca un archivo `font.ttf` y una carpeta `icon/icon.png` en el directorio del ejecutable para que el reproductor los cargue correctamente.

---

## 🎮 Controles

| Acción | Control |
| --- | --- |
| **Reproducir / Pausa** | `ESPACIO` o `CLIC IZQUIERDO` |
| **Siguiente Canción** | `FLECHA DERECHA` |
| **Canción Anterior** | `FLECHA IZQUIERDA` |
| **Subir / Bajar Volumen** | `FLECHAS ARRIBA/ABAJO` o `RUEDA DEL RATÓN` |
| **Abrir Carpeta** | `Tecla O` |
| **Modo Aleatorio (Shuffle)** | `Tecla R` |
| **Modo Bucle (Loop)** | `Tecla L` |
| **Buscar en la canción** | `CLIC` sobre la barra de progreso |

---

## 📂 Estructura del Código

El código se organiza en varios módulos clave:

* **PathManager:** Gestiona la creación de accesos directos en el sistema y rutas de iconos.
* **DiscordManager:** Maneja la comunicación con la API de Discord.
* **Reproductor:** Lógica central que gestiona la lista de reproducción, estados y carga de archivos mediante `std::filesystem`.
* **AudioThread:** Hilo dedicado para el refresco del buffer de audio, garantizando que la música no se corte incluso si la ventana se congela.

---

## 👤 Autor

Desarrollado por **Ars Byte** (RomanticHomicide).

---

**¿Te gustaría que añada una sección de "Solución de problemas" o instrucciones específicas para compilar el Discord RPC?**
