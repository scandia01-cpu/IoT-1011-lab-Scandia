"""Ejercicio 3. Monitor colaborativo: configura, observa, compara y exporta.
No hace falta escribir código Streamlit: conecta opciones ya preparadas.
"""
from datetime import datetime, timezone
import altair as alt
import pandas as pd
import requests
import streamlit as st

# === CONEXIONES DEL ESTUDIANTE ===
API_BASE = "https://iot-backen-1011.onrender.com"
EXPERIMENTO = "lab2026"
RUTA_MEDICIONES = "/mediciones"           # Conecta la fuente de datos del backend.
PROTOCOLO = "mqtt"                        # Monitor de la red del ejercicio 2.
ACTUALIZACION_SEG = 3                     # Frecuencia de consulta (segundos).
UMBRAL_DESCONEXION_SEG = 30               # Prueba despues con 15 y 45.
ACTIVAR_COMPARACION = False               # Cambia a True para conectar el comparador.
ACTIVAR_EXPORTACION = False               # Cambia a True para habilitar CSV.
# ================================

st.set_page_config(page_title="Red IoT | Laboratorio", layout="wide")
st.title("Monitoreo de la red IoT")
st.caption(f"MQTT publicado por los nodos → Render → consulta HTTP desde Streamlit. Actualización cada {ACTUALIZACION_SEG} s.")


def obtener_mediciones():
    respuesta = requests.get(
        API_BASE.rstrip("/") + RUTA_MEDICIONES,
        params={"experimento": EXPERIMENTO, "protocolo": PROTOCOLO, "limite": 1000},
        timeout=12,
    )
    respuesta.raise_for_status()
    return respuesta.json()["datos"]


def preparar_dataframe(datos):
    if not datos:
        return pd.DataFrame()
    df = pd.DataFrame(datos)
    df["hora_utc"] = pd.to_datetime(df["recibido_utc"], utc=True, errors="coerce")
    df = df.dropna(subset=["hora_utc", "nodo_id"])
    df = df.sort_values("hora_utc").copy()
    df["hora_chile"] = df["hora_utc"].dt.tz_convert("America/Santiago").dt.tz_localize(None)
    df["estacion"] = df["nodo_id"].astype(str) + " · " + df["zona"].astype(str)
    return df


def estados_desde_historial(df, ahora, umbral):
    if df.empty:
        return pd.DataFrame()
    ultimos = df.sort_values("hora_utc").groupby("nodo_id").tail(1).copy()
    ultimos["edad_s"] = (ahora - ultimos["hora_utc"]).dt.total_seconds().clip(lower=0)
    ultimos["estado"] = ultimos["edad_s"].apply(
        lambda x: "Reciente" if x <= umbral else "Sin datos recientes"
    )
    return ultimos.sort_values("nodo_id")


@st.fragment(run_every=ACTUALIZACION_SEG)
def actualizar():
    try:
        datos = obtener_mediciones()
        df = preparar_dataframe(datos)
    except (requests.RequestException, KeyError, ValueError) as err:
        st.error(f"No se pudo leer la API: {err}")
        st.caption("Revisa la conexión a Render; no significa que los sensores hayan fallado.")
        return

    if df.empty:
        st.info("Sin mediciones MQTT. Inicia dos Wokwi del ejercicio 2.")
        return

    disponibles = sorted(df["nodo_id"].unique().tolist())
    zonas = df.sort_values("hora_utc").drop_duplicates("nodo_id", keep="last").set_index("nodo_id")["zona"].to_dict()
    ahora = datetime.now(timezone.utc)

    st.subheader("1. Selecciona qué observar")
    nodos = st.multiselect(
        "Estaciones", disponibles, default=disponibles,
        format_func=lambda n: f"{n} · {zonas.get(n, '')}",
        key="seleccion_nodos",
    )
    a, b, c = st.columns([1, 1, 1])
    with a:
        variable = st.radio(
            "Variable", options=["temperatura_c", "humedad_pct"],
            format_func=lambda x: "Temperatura (°C)" if x == "temperatura_c" else "Humedad (%)",
            horizontal=True,
        )
    with b:
        ventana = st.select_slider("Últimos minutos", options=[5, 10, 20, 30, 60, 120], value=20)
    with c:
        suavizado = st.select_slider("Suavizado (lecturas)", options=[1, 3, 5, 10], value=1)

    if not nodos:
        st.warning("Selecciona al menos un nodo.")
        return

    # Estado calculado con todo el historial reciente recuperado, no solo con la ventana del gráfico.
    df_nodos = df[df["nodo_id"].isin(nodos)].copy()
    estado = estados_desde_historial(df_nodos, ahora, UMBRAL_DESCONEXION_SEG)
    en_ventana = df_nodos[df_nodos["hora_utc"] >= ahora - pd.Timedelta(minutes=ventana)].copy()

    total_recientes = int((estado["estado"] == "Reciente").sum()) if not estado.empty else 0
    m1, m2, m3 = st.columns(3)
    m1.metric("Nodos recientes", f"{total_recientes}/{len(nodos)}")
    m2.metric(f"Mediciones · últimos {ventana} min", len(en_ventana))
    m3.metric("Umbral sin datos", f"{UMBRAL_DESCONEXION_SEG} s")

    st.subheader("2. Evolución de las mediciones")
    if en_ventana.empty:
        st.warning("No hay mediciones en esta ventana; prueba con 60 o 120 minutos.")
    else:
        en_ventana["valor"] = pd.to_numeric(en_ventana[variable], errors="coerce")
        en_ventana["valor_mostrado"] = (
            en_ventana.groupby("nodo_id")["valor"]
            .transform(lambda serie: serie.rolling(suavizado, min_periods=1).mean())
        )
        # Una línea por nodo sobre el eje temporal real; no se pivotan instantes desalineados.
        grafico = alt.Chart(en_ventana).mark_line(point=True).encode(
            x=alt.X("hora_chile:T", title="Hora local (Chile)", axis=alt.Axis(format="%H:%M:%S")),
            y=alt.Y("valor_mostrado:Q", title="Temperatura (°C)" if variable == "temperatura_c" else "Humedad (%)",
                    scale=alt.Scale(zero=False)),
            color=alt.Color("estacion:N", title="Nodo · zona"),
            tooltip=[
                alt.Tooltip("estacion:N", title="Estación"),
                alt.Tooltip("hora_chile:T", title="Hora", format="%H:%M:%S"),
                alt.Tooltip("valor_mostrado:Q", title="Valor mostrado", format=".1f"),
                alt.Tooltip("secuencia:Q", title="Secuencia"),
            ],
        ).properties(height=350).interactive()
        st.altair_chart(grafico, use_container_width=True)
        st.caption(f"Cada línea corresponde a un nodo; los puntos son recepciones reales. "
                   f"Suavizado: promedio móvil de {suavizado} lectura(s) de cada nodo, no una medición nueva.")

    st.subheader("3. Estado y última lectura de cada nodo")
    tabla = estado[["nodo_id", "zona", "temperatura_c", "humedad_pct", "edad_s", "estado"]].copy()
    tabla = tabla.rename(columns={"nodo_id": "Nodo", "zona": "Zona", "temperatura_c": "Temp. (°C)",
                                  "humedad_pct": "Humedad (%)", "edad_s": "Seg. sin datos", "estado": "Estado"})
    st.dataframe(tabla, hide_index=True, use_container_width=True)

    if ACTIVAR_COMPARACION:
        st.subheader("4. Comparar dos nodos")
        if len(nodos) < 2:
            st.info("Selecciona dos nodos para comparar.")
        else:
            c1, c2 = st.columns(2)
            with c1:
                primero = st.selectbox("Nodo A", nodos, index=0)
            with c2:
                segundo = st.selectbox("Nodo B", nodos, index=1 if len(nodos) > 1 else 0)
            if primero == segundo:
                st.warning("Selecciona dos nodos distintos.")
            else:
                a_ult = estado[estado["nodo_id"] == primero].iloc[0]
                b_ult = estado[estado["nodo_id"] == segundo].iloc[0]
                desfase = abs((a_ult["hora_utc"] - b_ult["hora_utc"]).total_seconds())
                delta = float(a_ult[variable]) - float(b_ult[variable])
                unidad = "°C" if variable == "temperatura_c" else "%"
                st.metric(f"Última diferencia A − B ({variable})", f"{delta:+.1f} {unidad}")
                st.caption(f"Las lecturas no son necesariamente simultáneas: desfase {desfase:.1f} s."
                           " Si el desfase es grande, la comparación puede ser engañosa.")

    if ACTIVAR_EXPORTACION:
        st.subheader("5. Exportar datos de la ventana")
        csv = en_ventana[["recibido_utc", "nodo_id", "zona", "temperatura_c", "humedad_pct", "secuencia"]].to_csv(index=False)
        st.download_button("Descargar CSV del experimento", data=csv, file_name="lab2026_lecturas.csv", mime="text/csv")


actualizar()
