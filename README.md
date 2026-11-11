# Taller IoT 1011 · HTTP y MQTT — Scandia

Solución del taller de 3 ejercicios (Wokwi + Render + Streamlit), basada en el material original del curso.
Solo se cambiaron **parámetros**, nunca funciones de librerías.

| Ejercicio | Carpeta | Qué se configuró |
|---|---|---|
| 1 · HTTP vs MQTT | `01_HTTP_vs_MQTT/` | `NODO_ID = "N01_scandia"`, `RONDA = "R01"` en ambos sketches |
| 2 · Red MQTT | `02_Red_MQTT/` | `N01_scandia` (exterior), `N02_scandia` (interior), `N03_scandia` (ventilación, opcional) + variantes de los pasos 5 y 6 en `variantes/` |
| 3 · Streamlit | `03_Streamlit/` | `app.py` original + variantes de los pasos 5, 6 y 7 en `variantes/` |

- **Cómo ejecutarlo paso a paso:** [`GUIA_PASO_A_PASO.md`](GUIA_PASO_A_PASO.md)
- **Resultados y respuestas:** [`INFORME.md`](INFORME.md) (completar con las mediciones propias)
- Instrucciones originales del curso: `LEER_PRIMERO.md` y el `README.md` de cada carpeta.

Servidor: `https://iot-backen-1011.onrender.com` · Experimento: `lab2026`

## Evidencias

Carpeta `evidencias/` (16 capturas + CSV):

- **Ejercicio 1:** `ej1_http_envios.png`, `ej1_http_error_422.png`, `ej1_docs_post_prueba_201.png`, `ej1_http_metricas.png`, `ej1_mqtt_metricas.png`
- **Ejercicio 2:** `ej2_publish_recibido_N01.png`, `ej2_publish_recibido_companera.png`
- **Ejercicio 3:** `ej3_curvas_temperatura.png`, `ej3_curvas_humedad.png`, `ej3_intervalo2000_suavizado1.png`, `ej3_suavizado5.png`, `ej3_curvas_dos_nodos.png`, `ej3_comparador.png`, `ej3_comparador_reciente.png`, `ej3_estado.png`, `ej3_estado_umbral15.png`, `ej3_lab2026_lecturas.csv`
