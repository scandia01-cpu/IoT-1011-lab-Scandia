# Ejercicio 2 · Red MQTT compartida

**Objetivo:** dos o tres ESP32 publican datos diferentes y reciben los mensajes de los demás. Los nodos forman parte de la misma red.

1. Se abren dos proyectos Wokwi con `diagram.json` y `libraries.txt`. El primer estudiante utiliza `N01_Exterior.ino` y el segundo, `N02_Interior.ino`. El archivo `N03_Ventilacion.ino` es opcional.
2. En cada sketch se cambia `NODO_ID` (por ejemplo, `N01_luis` y `N02_ana`). Se mantienen `INTERVALO = 5000` y `NODO_A_ESCUCHAR = "+"`.
3. Se inician ambos nodos. En los dos monitores seriales se buscan `PUBLISH=SI` y `RECIBIDO ... DE`, y se observan los tópicos que aparecen en `PUBLICO EN` y `ESCUCHO`.
4. En uno de los Wokwi se hace clic sobre el DHT22 y se aumenta la temperatura 8 °C. El cambio debe comprobarse en el monitor **del otro estudiante**. Después, los participantes intercambian los roles.
5. Se cambia `INTERVALO` a **2000** en el primer nodo y a **7000** en el segundo; se reinician ambos y se observa durante un minuto cuál publica más. No debe modificarse el código de las funciones MQTT.
6. En el primer nodo se sustituye `NODO_A_ESCUCHAR = "+"` por el ID exacto del segundo, por ejemplo `"N02_ana"`. Se reinicia y se verifica qué mensajes recibe. Posteriormente, se restablece el valor `"+"`.
7. Se detiene el segundo nodo durante 30 segundos y luego se vuelve a iniciar. Se consulta la página de [nodos en Render](https://iot-backen-1011.onrender.com/nodos?experimento=lab2026) y se observa qué cambia.

**Cada estudiante responde en 4–6 líneas:** ¿Por qué `+` permite escuchar varios emisores? ¿Qué diferencia hay entre publicar y suscribirse? ¿Qué cambió al utilizar dos intervalos distintos? ¿Al reconectar llegaron los mensajes enviados durante la desconexión o solo los nuevos? ¿Por qué el nodo exterior y el interior no muestran exactamente lo mismo?

**Evidencia:** una captura de `PUBLISH=SI` y `RECIBIDO ... DE`, indicando qué intervalos y filtro se probaron.

*Los tres perfiles son mediciones sintéticas del DHT22 simulado con variaciones de software: no representan un invernadero real calibrado.*
