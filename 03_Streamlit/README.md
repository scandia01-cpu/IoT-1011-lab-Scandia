# Ejercicio 3 · Monitoreo y comparación con Streamlit

**Contexto:** varios nodos publican datos distintos; el monitor del estudiante debe mostrar qué cambia, qué nodo dejó de enviar y la diferencia entre estaciones.

1. Se mantienen en funcionamiento **N01 y N02** del ejercicio 2. En `app.py` se revisan `API_BASE`, `RUTA_MEDICIONES` y `EXPERIMENTO`. No deben modificarse las funciones.
2. En esta carpeta se ejecutan `python -m pip install -r requirements.txt` y `python -m streamlit run app.py`.
3. Se seleccionan **temperatura**, **20 minutos** y **suavizado 1**. Se modifica el DHT22 de N01 y se observa su línea identificada como `N01 · exterior`. El procedimiento se repite con **humedad**.
4. Se cambia el intervalo de envío de N01 en Wokwi y se observa si aparecen más o menos puntos por minuto. Se prueban **suavizado 1 y 5** para determinar qué detalle se pierde al suavizar.
5. En `app.py` se cambia `ACTIVAR_COMPARACION = True`. De regreso en el navegador, se seleccionan dos nodos y se observan la **diferencia A − B** y el desfase entre sus últimas lecturas.
6. Se detiene N02 durante más de 30 segundos y se comprueba el mensaje `Sin datos recientes`. Luego se cambia `UMBRAL_DESCONEXION_SEG` a **15** y se observa la diferencia. Finalmente, se restablece el valor **30**.
7. Se activa `ACTIVAR_EXPORTACION = True`, se exporta el CSV y se comprueba que contenga el ID, la zona, la hora y las mediciones de los dos nodos.

**El estudiante debe responder:** ¿Qué representan los puntos y las líneas? ¿Por qué importa el desfase de tiempo al comparar dos nodos? ¿Qué efecto tiene suavizar los datos? ¿Cómo se diferencia una estación desconectada de una que transmite una temperatura constante? ¿Qué aporta la exportación CSV a un análisis posterior?

**Evidencia:** captura con las dos curvas identificadas, el comparador y la tabla de estado. No se requiere construir gráficos desde cero.
