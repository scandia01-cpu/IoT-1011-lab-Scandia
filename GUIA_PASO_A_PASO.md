# Guía paso a paso (Wokwi + Streamlit)

> En Wokwi, cada proyecto ESP32 tiene tres pestañas: `sketch.ino`, `diagram.json` y `libraries.txt`.
> Para crear `libraries.txt`: en el editor, flecha ▾ junto a las pestañas → **New file…** → `libraries.txt`
> (o usa el *Library Manager* y agrega: *DHT sensor library for ESPx*, *ArduinoJson*, *WebSockets*).
> Inicia sesión en Wokwi para poder guardar los proyectos (**Save**) y abrir varios a la vez.
> Render puede tardar ~1 minuto en "despertar" la primera vez: si el primer envío falla, espera y sigue.

---

## Ejercicio 1 · HTTP vs MQTT (≈10 min)

1. Abre <https://wokwi.com/projects/new/esp32>.
2. Reemplaza `diagram.json` por el de `01_HTTP_vs_MQTT/diagram.json` y crea `libraries.txt` con el contenido de `01_HTTP_vs_MQTT/libraries.txt`.
3. Reemplaza `sketch.ino` con **todo** `01_HTTP_vs_MQTT/01_HTTP.ino` (ya tiene `N01_scandia` y `R01`). ▶ Play.
4. Mira el Serial Monitor: deben aparecer 30 líneas `HTTP n/30 | codigo=201 | JSON=… B`. Cuenta cuántas tienen `codigo=201`.
5. Al terminar, el sketch imprime `METRICAS HTTP GET: 200` y las métricas. También puedes abrir:
   <https://iot-backen-1011.onrender.com/metricas/serial?experimento=lab2026&nodo_id=N01_scandia&ronda=R01>
   📸 Captura → `evidencias/ej1_http_metricas.png`.
6. Detén la simulación. Reemplaza `sketch.ino` por `01_HTTP_vs_MQTT/02_MQTT_WSS.ino` (mismo ID y ronda). ▶ Play.
7. Deben aparecer 30 líneas `MQTT n/30 | PUBLISH=SI | JSON=… B` y luego `METRICAS EN RENDER`. Vuelve a abrir el enlace.
   📸 Captura → `evidencias/ej1_mqtt_metricas.png`.
8. Copia los valores en la tabla del `INFORME.md`.

**Si necesitas repetir:** cambia `RONDA = "R01"` por `"R02"` en **ambos** sketches (y en el enlace usa `ronda=R02`).

---

## Ejercicio 2 · Red MQTT (dos pestañas de Wokwi)

Abre **dos** proyectos Wokwi ESP32 (dos pestañas del navegador), ambos con `02_Red_MQTT/diagram.json` y `02_Red_MQTT/libraries.txt`:

| Pestaña | `sketch.ino` |
|---|---|
| A (N01) | `02_Red_MQTT/N01_Exterior.ino` |
| B (N02) | `02_Red_MQTT/N02_Interior.ino` |
| C (opcional) | `02_Red_MQTT/N03_Ventilacion.ino` |

> Wokwi puede pausar la simulación de una pestaña que no está visible. Si un nodo deja de publicar,
> pon las pestañas en **ventanas separadas lado a lado** en vez de pestañas de la misma ventana.

| Paso | Qué hacer | Qué observar / evidencia |
|---|---|---|
| 3 | ▶ Play en A y B | En cada monitor: `PUBLICO EN: doctorado/lab2026/N0x_Scandia/telemetria`, `ESCUCHO: doctorado/lab2026/+/telemetria`, líneas `PUBLISH=SI` y `RECIBIDO n DE N0y_Scandia`. 📸 `ej2_publish_recibido_N01.png` y `…_N02.png` |
| 4 | En A, clic sobre el DHT22 → sube la temperatura **8 °C** (24 → 32) | En el monitor de **B**, `RECIBIDO … DE N01_scandia` muestra T ≈ 8 °C más alta. Luego al revés (sube en B, mira en A) |
| 5 | A ← `variantes/paso5_N01_intervalo2000.ino`; B ← `variantes/paso5_N02_intervalo7000.ino`; reinicia ambos y observa **1 minuto** | Cuenta las publicaciones de cada uno en 60 s (≈30 vs ≈8–9) |
| 6 | A ← `variantes/paso6_N01_escucha_solo_N02.ino`; reinicia A (deja C encendido si lo usas) | `ESCUCHO: doctorado/lab2026/N02_scandia/telemetria` → A solo recibe de N02. Después vuelve A a `N01_Exterior.ino` (`"+"`) |
| 7 | Detén B ⏹ 30 s y vuelve a ▶ | Abre <https://iot-backen-1011.onrender.com/nodos?experimento=lab2026>, compara antes/después. 📸 `ej2_nodos_render.png`. En A: ¿llegan los mensajes del período detenido o solo los nuevos? |

Anota en `INFORME.md` qué intervalos y qué filtro probaste.

---

## Ejercicio 3 · Streamlit (en tu computador)

Mantén **N01 y N02 corriendo** en Wokwi (con `N01_Exterior.ino` y `N02_Interior.ino`).

```bash
cd 03_Streamlit
python -m pip install -r requirements.txt
python -m streamlit run app.py
```

Se abre <http://localhost:8501>.

| Paso | Qué hacer | Evidencia / observar |
|---|---|---|
| 3 | Temperatura · 20 min · suavizado 1. Cambia el DHT22 de N01 → la línea `N01_scandia · exterior` salta. Repite con humedad | 📸 `ej3_curvas.png` (dos curvas identificadas) |
| 4 | En Wokwi cambia el intervalo de N01 (usa `paso5_N01_intervalo2000.ino`) → más puntos por minuto. Prueba suavizado 1 y 5 | Con 5 se aplanan los picos y el salto se ve más tarde/más suave |
| 5 | Reemplaza `app.py` por `variantes/app_paso5_comparacion.py` (o cambia `ACTIVAR_COMPARACION = True`) y guarda; Streamlit recarga | Nodo A = N01, Nodo B = N02 → "Última diferencia A − B" y "desfase … s". 📸 `ej3_comparador.png` |
| 6 | Detén N02 > 30 s → tabla muestra `Sin datos recientes`. Luego usa `variantes/app_paso6_umbral15.py` (umbral 15) → el aviso aparece antes. Vuelve a 30 | 📸 `ej3_estado.png` |
| 7 | Usa `variantes/app_paso7_exportacion.py` (comparación + exportación, umbral 30) → **Descargar CSV** | Revisa que tenga `nodo_id`, `zona`, `recibido_utc`, `temperatura_c`, `humedad_pct` de los dos nodos. Guárdalo como `evidencias/ej3_lab2026_lecturas.csv` |

> Para usar una variante: copia su contenido dentro de `app.py` (o ejecuta directamente
> `python -m streamlit run variantes/app_paso7_exportacion.py`).

---

## Subir a GitHub

**Opción web (sin terminal):** en GitHub → **New repository** → nombre `IoT-1011-lab-Scandia` → Create →
**uploading an existing file** → arrastra todo el contenido de esta carpeta (incluida `evidencias/`) → **Commit changes**.

**Opción terminal:**

```bash
cd IoT-1011-lab-Scandia
git init -b main
git add .
git commit -m "Taller IoT 1011: ejercicios 1-3"
git remote add origin https://github.com/<tu-usuario>/IoT-1011-lab-Scandia.git
git push -u origin main
```

Después de completar `INFORME.md` y agregar las capturas: `git add . && git commit -m "Resultados y evidencias" && git push`.
