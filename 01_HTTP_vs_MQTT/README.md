# Ejercicio 1 · Comparar HTTP y MQTT

**Objetivo:** medir el tamaño de los mensajes y su recepción para dos protocolos con el **mismo JSON**, sin construir las librerías.

1. El estudiante abre Wokwi ESP32 y copia `diagram.json`, `libraries.txt` y `01_HTTP.ino` como `sketch.ino`.
2. Se cambia `NODO_ID` a `N01_tunombre`, sustituyendo `tunombre` por el nombre real del estudiante, sin espacios ni tildes. Se conserva `RONDA = "R01"`.
3. Se ejecutan 30 envíos HTTP y se registra cuántos intentos tuvieron `codigo=201`. Las métricas calculadas en Render pueden consultarse desde el Serial Monitor o mediante [este enlace](https://iot-backen-1011.onrender.com/metricas/serial?experimento=lab2026&nodo_id=N01_tunombre&ronda=R01), sustituyendo el ID por el propio.
4. Se reemplaza `sketch.ino` por `02_MQTT_WSS.ino`, con el **mismo** `NODO_ID`, `RONDA` y servidor, y se ejecutan 30 publicaciones. Se consulta nuevamente el enlace.
5. Deben copiarse los resultados propios, no los de otro nodo. Si se repite la prueba, se cambia la ronda a `R02` **en ambos sketches** para evitar registros duplicados.

| Métrica (calculada en Render) | HTTP | MQTT |
|---|---|---|
| Intentos en Wokwi | 30 | 30 |
| Mediciones únicas recibidas | | |
| Bytes de JSON | | |
| Tamaño estimado de mensajes | | |
| Sobrecarga estimada | | |
| Eficiencia estimada (%) | | |

**El estudiante debe responder:** ¿Llegaron las 30 mediciones por ambos protocolos? ¿Cuál generó menos bytes de mensaje estimados y por qué? ¿Qué parte de esta medición corresponde realmente al JSON? ¿Un `PUBLISH=SI` equivale a una medición recibida? ¿Por qué no es posible concluir cuál protocolo consume menos batería a partir de estos resultados?

**Límite del experimento:** el backend estima el tamaño de las solicitudes HTTP recibidas y del paquete MQTT PUBLISH. **No mide** Wi-Fi, TCP/IP, TLS, WebSocket, conexiones, energía ni costo monetario. Se trata de una comparación de sobrecarga *estimada*, no de tráfico real total. HTTP y MQTT recorren transportes distintos, por lo que sus latencias no deben compararse como si fueran equivalentes.
