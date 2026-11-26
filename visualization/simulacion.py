"""Reproducción de un circuito de reparto calculado por el planificador C."""
from time import monotonic

import numpy as np
from matplotlib.widgets import Button

FONDO = "#f3f5f4"
PANEL = "#142d37"
TINTA = "#173541"
CLARO = "#f6faf9"
SUAVE = "#b6ced0"
RUTA = "#ce542e"
PENDIENTE = "#ad6816"
COMPLETA = "#128264"
REPARTIDOR = "#126abe"


class Recorrido:
    def __init__(self, plan, segmentos, coords):
        self.tramos = plan["tramos"]
        self.inicio = plan["salida_min"]
        self.fin = self.tramos[-1]["servicio_min"]
        self.entregas = {e["id"]: e for e in plan["entregas_por_prioridad"]}
        self.servicios = [t["servicio_min"] for t in self.tramos]
        self.caminos = []
        offset = 0
        for tramo in self.tramos:
            origen = coords.loc[tramo["origen"]].to_numpy(dtype=float)
            partes = [origen[None, :]]
            for _ in tramo["aristas"]:
                puntos = np.asarray(segmentos[offset])
                offset += 1
                if np.linalg.norm(puntos[-1] - partes[-1][-1]) < np.linalg.norm(puntos[0] - partes[-1][-1]):
                    puntos = puntos[::-1]
                if not np.allclose(puntos[0], partes[-1][-1], atol=2e-6, rtol=0):
                    raise ValueError("Geometría OSM discontinua en el recorrido")
                partes.append(puntos[1:] if len(puntos) > 1 else puntos)
            puntos = np.concatenate(partes)
            delta = np.diff(puntos, axis=0)
            delta[:, 0] *= np.cos(np.deg2rad(origen[1]))
            dist = np.r_[0., np.cumsum(np.linalg.norm(delta, axis=1))]
            self.caminos.append((puntos, dist))

    def posicion(self, minuto):
        i = min(int(np.searchsorted(self.servicios, minuto, side="right")), len(self.tramos)-1)
        inicio = self.servicios[i-1] if i else self.inicio
        duracion = self.tramos[i]["llegada_min"] - inicio
        fraccion = np.clip((minuto-inicio)/duracion, 0, 1) if duracion else 1
        puntos, dist = self.caminos[i]
        xy = [np.interp(fraccion * dist[-1], dist, puntos[:, k]) for k in (0, 1)]
        return i, xy


class Simulacion:
    def __init__(self, fig, ax, plan, segmentos, coords):
        self.fig, self.ax = fig, ax
        self.recorrido = Recorrido(plan, segmentos, coords)
        self.coords = coords
        self.minuto = self.recorrido.inicio
        self.estado = "Lista"
        self.velocidades = (0.5, 1, 2, 4, 8)
        self.indice_vel = 1
        self.ultimo = None
        self.fondo = None
        self.arrastre = False
        self.botones = {}
        fig.set_facecolor(FONDO)
        fig.set_size_inches(13.2, 7.6)
        fig.canvas.manager.set_window_title("Reparto · Tijuana")
        fig.subplots_adjust(left=.055, right=.695, bottom=.13, top=.89)
        fig.text(.055, .93, "Ruta de reparto", fontsize=22, weight="bold", color=TINTA)
        ax.set_title("Tijuana  /  OpenStreetMap", loc="left", fontsize=10, color=TINTA)
        ax.tick_params(labelsize=8, colors="#547079")
        ax.get_legend().remove()
        ax.set_facecolor("#fcfcf8")
        self.limites = ax.get_xlim(), ax.get_ylim()
        panel = fig.add_axes([.72, 0, .28, 1], facecolor=PANEL)
        panel.set_xticks([])
        panel.set_yticks([])
        for borde in panel.spines.values():
            borde.set_visible(False)

        def texto(x, y, valor, size=11, color=CLARO, **kwargs):
            return fig.text(x, y, valor, fontsize=size, color=color, **kwargs)

        texto(.747, .944, "Simulación", 20, weight="bold")
        texto(.747, .897, "DISTANCIA TOTAL", 9, SUAVE)
        texto(.876, .897, "TIEMPO ESTIMADO", 9, SUAVE)
        self.dist_txt = texto(.747, .855, f"{plan['distancia_m']/1000:.2f} km", 16)
        self.duracion_txt = texto(.876, .855, f"{self.recorrido.fin-self.recorrido.inicio} min", 16)
        self.estado_txt = texto(.747, .81, "")
        self.hora_txt = texto(.935, .81, "", ha="right")
        self.entrega_txt = texto(.747, .762, "", 16, weight="bold")
        self.prio_txt = texto(.747, .725, "", 11, SUAVE)
        self.cuenta_txt = texto(.747, .69, "")
        
        texto(.747, .497, "Velocidad", 10)
        self.vel_txt = texto(.835, .497, "", 12)
        texto(.055, .048, "Rueda para acercar o alejar   ·   Arrastra para mover el mapa", 10, TINTA)
        self.actual, = ax.plot([], [], marker="o", ms=8, color=REPARTIDOR,
                               mec="white", mew=1.3, ls="", zorder=8)
        self.hechas = ax.scatter([], [], marker="o", s=83, facecolor=COMPLETA,
                                 edgecolor="white", linewidth=1.3, zorder=6)
        ax.legend(handles=[
            self.muestra("D", TINTA, "Depósito"), self.muestra("o", PENDIENTE, "Pendiente"),
            self.muestra("o", COMPLETA, "Completada"), self.muestra("o", REPARTIDOR, "Repartidor")
        ], loc="lower left", ncol=2, fontsize=8, framealpha=.95)
        self.dinamicos = [self.hechas, self.actual, self.estado_txt, self.hora_txt,
                          self.entrega_txt, self.prio_txt, self.cuenta_txt, self.vel_txt]
        texto(.747, .426, "Gestión de entregas", 15, weight="bold")
        self.estados_entregas = {}
        for fila, e in enumerate(sorted(self.recorrido.entregas.values(), key=lambda e: e["id"])):
            y = .365 - fila * .093
            texto(.747, y, f"Entrega {e['id']}", 11, weight="bold")
            prioridad = e["prioridad"]
            texto(.973, y, ("Baja", "Media", "Alta")[prioridad], 10,
                  ("#76d5ab", "#ffc66b", "#ff9395")[prioridad], ha="right")
            desde, hasta = e["desde_min"], e["hasta_min"]
            texto(.747, y-.032, f"{desde//60:02}:{desde%60:02} – {hasta//60:02}:{hasta%60:02}", 10, SUAVE)
            self.estados_entregas[e["id"]] = texto(.973, y-.032, "", 10, SUAVE, ha="right")
        self.dinamicos.extend(self.estados_entregas.values())
        for artista in self.dinamicos:
            artista.set_animated(fig.canvas.supports_blit)

        self.boton("Iniciar", .747, .63, self.iniciar)
        self.boton("Pausar", .868, .63, self.pausar)
        self.boton("Continuar", .747, .575, self.continuar)
        self.boton("Reiniciar", .868, .575, self.reiniciar)
        self.boton("−", .887, .487, lambda: self.velocidad(-1), .037)
        self.boton("+", .934, .487, lambda: self.velocidad(1), .037)
        self.boton("Ver mapa completo", .747, .531, self.encuadrar, .232)
        self.timer = fig.canvas.new_timer(interval=40)
        self.timer.add_callback(self.paso)
        for nombre, funcion in [
            ("draw_event", self.al_dibujar), ("scroll_event", self.zoom),
            ("button_press_event", self.presionar), ("motion_notify_event", self.mover),
            ("button_release_event", self.soltar), ("close_event", lambda _: self.timer.stop())
        ]:
            fig.canvas.mpl_connect(nombre, funcion)
        self.actualizar()

    @staticmethod
    def muestra(marker, color, label):
        from matplotlib.lines import Line2D
        return Line2D([], [], marker=marker, color=color, ls="", label=label)

    def boton(self, nombre, x, y, accion, ancho=.111):
        eje = self.fig.add_axes([x, y, ancho, .038])
        b = Button(eje, nombre, color="#254854", hovercolor="#36616d", useblit=False)
        b.label.set_color(CLARO)
        b.label.set_fontsize(10)
        for borde in eje.spines.values():
            borde.set_edgecolor("#486975")
        b.on_clicked(lambda _: accion())
        self.botones[nombre] = b

    def al_dibujar(self, evento):
        if evento.canvas.supports_blit:
            self.fondo = evento.canvas.copy_from_bbox(self.fig.bbox)
            for artista in self.dinamicos:
                self.fig.draw_artist(artista)

    def actualizar(self):
        r = self.recorrido
        i, xy = r.posicion(self.minuto)
        t = r.tramos[i]
        terminadas = [s for s in r.tramos if s["entrega_id"] is not None and
                      s["servicio_min"] <= self.minuto and self.estado != "Lista"]
        self.hechas.set_offsets(np.array([self.coords.loc[s["destino"]].values for s in terminadas]).reshape(-1, 2))
        self.actual.set_data([xy[0]], [xy[1]])
        e = r.entregas.get(t["entrega_id"])
        self.estado_txt.set_text(self.estado)
        self.hora_txt.set_text(f"{int(self.minuto)//60:02}:{int(self.minuto)%60:02}")
        self.entrega_txt.set_text(f"Visita {i+1} · Entrega {e['id']}" if e else
                                 ("En el depósito" if self.estado == "Finalizada" else "Regreso al depósito"))
        self.prio_txt.set_text("Prioridad " + ("baja", "media", "alta")[e["prioridad"]] if e else "")
        self.cuenta_txt.set_text(f"Completadas   {len(terminadas)} / {len(r.entregas)}")
        self.vel_txt.set_text(f"{self.velocidades[self.indice_vel]:g}×")
        completadas = {t["entrega_id"] for t in terminadas}
        for identificador, etiqueta in self.estados_entregas.items():
            etiqueta.set_text("Completada" if identificador in completadas else "Pendiente")
        if self.fondo is None:
            self.fig.canvas.draw_idle()
        else:
            self.fig.canvas.restore_region(self.fondo)
            for artista in self.dinamicos:
                self.fig.draw_artist(artista)
            self.fig.canvas.blit(self.fig.bbox)

    def iniciar(self):
        if self.estado == "Lista":
            self.arrancar()

    def continuar(self):
        if self.estado == "En pausa":
            self.arrancar()

    def arrancar(self):
        self.estado = "En marcha"
        self.ultimo = monotonic()
        self.timer.start()
        self.paso()

    def paso(self):
        if self.estado != "En marcha":
            return
        ahora = monotonic()
        # A 1x, un segundo de reproducción representa un minuto del plan.
        self.minuto = min(self.recorrido.fin, self.minuto + (ahora-self.ultimo)*self.velocidades[self.indice_vel])
        self.ultimo = ahora
        if self.minuto >= self.recorrido.fin:
            self.estado = "Finalizada"
            self.timer.stop()
        self.actualizar()

    def pausar(self):
        if self.estado == "En marcha":
            self.paso()
            if self.estado != "Finalizada":
                self.estado = "En pausa"
            self.timer.stop()
            self.actualizar()

    def reiniciar(self):
        self.timer.stop()
        self.minuto = self.recorrido.inicio
        self.estado = "Lista"
        self.actualizar()

    def velocidad(self, cambio):
        self.paso()
        self.indice_vel = max(0, min(len(self.velocidades)-1, self.indice_vel+cambio))
        self.actualizar()

    def encuadrar(self):
        self.ax.set_xlim(self.limites[0])
        self.ax.set_ylim(self.limites[1])
        self.fig.canvas.draw_idle()

    def zoom(self, e):
        if e.inaxes is not self.ax or e.xdata is None or e.ydata is None:
            return
        escala = 1.25 ** -np.clip(e.step, -5, 5)
        for centro, limites, poner in [(e.xdata, self.ax.get_xlim(), self.ax.set_xlim),
                                       (e.ydata, self.ax.get_ylim(), self.ax.set_ylim)]:
            poner(centro + (np.array(limites)-centro)*escala)
        self.fig.canvas.draw_idle()

    def presionar(self, e):
        if e.inaxes is self.ax and e.button == 1 and not self.ax.get_navigate_mode():
            self.ax.start_pan(e.x, e.y, 1)
            self.arrastre = True

    def mover(self, e):
        if self.arrastre:
            self.ax.drag_pan(1, None, e.x, e.y)
            self.fig.canvas.draw_idle()

    def soltar(self, _):
        if self.arrastre:
            self.ax.end_pan()
            self.arrastre = False

