En Control de cambios hay un ejecutable para rellenar el fichero de forma facil

```bash
./control_cambios <parte (-s|-h|-p|-m|software|hardware|planificacion|memoria)>
<tipo (-a|-c|-m|anadido|cambio|modificacion)>
```


🗂️ Fases y hitos

Fase 1 — Especificación del proyecto (2025-09-16 → 2025-09-29)
Hito H1: Especificación aprobada (2025-09-29).

Fase 2 — Diseño electrónico y captura esquemática (2025-09-30 → 2025-10-13)
Hito H2: Esquemático congelado y BOM v1 (2025-10-13).

Fase 3 — Diseño PCB (2025-10-14 → 2025-10-27)
Hito H3: Gerbers v1 listos para fabricación (2025-10-27).

Fase 4 — Fabricación y montaje (2025-10-28 → 2025-11-10)
Hito H4: Placas recibidas y montadas (2025-11-10).

Fase 5 — Firmware: simulación y depuración (2025-11-11 → 2025-11-24)
Hito H5: Firmware MVP funcional en HW (2025-11-24).

Fase 6 — Verificación hardware (2025-11-25 → 2025-12-08)
Hito H6: Validación de prestaciones y montaje final (2025-12-08).

Fase 7 — Reserva / Cierre (2025-12-09 → 2025-12-22)
Hito H7: Cierre de proyecto y entrega final (2025-12-22).

📅 Plan detallado (por tareas)

Todas las tareas tienen duración 7 días.
Formato: ID — Nombre (Inicio → Fin)

1) Especificación (2025-09-16 → 2025-09-29)

1 — Especificación del proyecto: Definición de objetivos (2025-09-16 → 2025-09-22)

2 — Especificación del proyecto: Estudio sistema (2025-09-23 → 2025-09-29)

2) Diseño electrónico y esquemático (2025-09-30 → 2025-10-13)

3 — Búsqueda de componentes (2025-09-30 → 2025-10-06)

4 — Posicionado componentes (2025-09-30 → 2025-10-06)

5 — Creación de nuevos componentes (2025-09-30 → 2025-10-06)

6 — Documentación esquemático. Control de cambios (2025-09-30 → 2025-10-06)

7 — Análisis consumo eléctrico (2025-09-30 → 2025-10-06)

8 — Análisis disipación de potencia (2025-09-30 → 2025-10-06)

9 — Rutado de conexiones (2025-09-30 → 2025-10-06)

10 — Generación de listado de materiales (BOM) (2025-09-30 → 2025-10-06)

11 — Búsqueda de componentes (2025-10-07 → 2025-10-13)

12 — Posicionado componentes (2025-10-07 → 2025-10-13)

13 — Creación de nuevos componentes (2025-10-07 → 2025-10-13)

14 — Documentación esquemático. Control de cambios (2025-10-07 → 2025-10-13)

15 — Análisis consumo eléctrico (2025-10-07 → 2025-10-13)

16 — Análisis disipación de potencia (2025-10-07 → 2025-10-13)

17 — Rutado de conexiones (2025-10-07 → 2025-10-13)

18 — Generación de listado de materiales (BOM) (2025-10-07 → 2025-10-13)

3) Diseño de PCB (2025-10-14 → 2025-10-27)

19 — Creación de huellas de nuevos componentes (2025-10-14 → 2025-10-20)

20 — Posicionado de componentes (2025-10-14 → 2025-10-20)

21 — Definición de borde y zonas de conectores (2025-10-14 → 2025-10-20)

22 — Rutado de conexiones (2025-10-14 → 2025-10-20)

23 — Planos de disipación térmica (2025-10-14 → 2025-10-20)

24 — Planos de masa y otros (2025-10-14 → 2025-10-20)

25 — Documentación: serigrafía. Control de cambios (2025-10-14 → 2025-10-20)

26 — Documentación: acotaciones geométricas (2025-10-14 → 2025-10-20)

27 — Generación de ficheros para el fabricante (gerber) (2025-10-14 → 2025-10-20)

28 — Creación de huellas de nuevos componentes (2025-10-21 → 2025-10-27)

29 — Posicionado de componentes (2025-10-21 → 2025-10-27)

30 — Definición de borde y zonas de conectores (2025-10-21 → 2025-10-27)

31 — Rutado de conexiones (2025-10-21 → 2025-10-27)

32 — Planos de disipación térmica (2025-10-21 → 2025-10-27)

33 — Planos de masa y otros (2025-10-21 → 2025-10-27)

34 — Documentación: serigrafía. Control de cambios (2025-10-21 → 2025-10-27)

35 — Documentación: acotaciones geométricas (2025-10-21 → 2025-10-27)

36 — Generación de ficheros para el fabricante (gerber) (2025-10-21 → 2025-10-27)

4) Fabricación y montaje (2025-10-28 → 2025-11-10)

37 — Envío al fabricante (2025-10-28 → 2025-11-03)

38 — Recepción y revisión (2025-10-28 → 2025-11-03)

39 — Montaje de componentes (2025-10-28 → 2025-11-03)

40 — Envío al fabricante (2025-11-04 → 2025-11-10)

41 — Recepción y revisión (2025-11-04 → 2025-11-10)

42 — Montaje de componentes (2025-11-04 → 2025-11-10)

5) Firmware: simulación y depuración (2025-11-11 → 2025-11-24)

43 — Control de elementos hardware: drivers (2025-11-11 → 2025-11-17)

44 — Verificación (2025-11-11 → 2025-11-17)

45 — Programa principal. Especificaciones (2025-11-11 → 2025-11-17)

46 — Documentación firmware. Control de cambios (2025-11-11 → 2025-11-17)

47 — Control de elementos hardware: drivers (2025-11-18 → 2025-11-24)

48 — Verificación (2025-11-18 → 2025-11-24)

49 — Programa principal. Especificaciones (2025-11-18 → 2025-11-24)

50 — Documentación firmware. Control de cambios (2025-11-18 → 2025-11-24)

6) Verificación hardware (2025-11-25 → 2025-12-08)

51 — Verificación inicial (2025-11-25 → 2025-12-01)

52 — Verificación firmware (2025-11-25 → 2025-12-01)

53 — Resolución de problemas (2025-11-25 → 2025-12-01)

54 — Análisis de prestaciones (2025-11-25 → 2025-12-01)

55 — Montaje final. Piezas mecánicas (2025-11-25 → 2025-12-01)

56 — Verificación inicial (2025-12-02 → 2025-12-08)

57 — Verificación firmware (2025-12-02 → 2025-12-08)

58 — Resolución de problemas (2025-12-02 → 2025-12-08)

59 — Análisis de prestaciones (2025-12-02 → 2025-12-08)

60 — Montaje final. Piezas mecánicas (2025-12-02 → 2025-12-08)

7) Reserva / Cierre (2025-12-09 → 2025-12-22)

61 — (Reserva / buffer) (2025-12-09 → 2025-12-15)

62 — (Reserva / buffer) (2025-12-16 → 2025-12-22)

📦 Entregables por fase

H1 — Especificación: Documento de requisitos, casos de uso, interfaces, criterios de aceptación.

H2 — Esquemático + BOM: Esquemático revisado, librerías de símbolos, BOM v1 con alternativos.

H3 — PCB lista para fabricación: Ficheros Gerber/ODB++, taladros (NC-Drill), pick&place, drawing de fabricación, revisión DFM.

H4 — Lote de PCBs: Placas recibidas, informe de inspección (óptica/continuidad), placas montadas o lista para montaje.

H5 — Firmware MVP: Drivers básicos, arranque, pruebas de IO, plan de pruebas y reporte.

H6 — Validación: Informe de prestaciones (eléctrico/térmico), checklist de montaje final, manual breve de ensamblaje.

H7 — Cierre: Dossier final, versión congelada de ficheros CAD/firmware, release etiquetada.

✅ Definición de “hecho” (DoD)

Especificación: Aprobación de stakeholders, trazabilidad requisito-prueba.

Esquemático: Revisión por pares + ERC limpio + BOM con stock/lead-time comprobado.

PCB: DRC a 0, revisión térmica, revisión mecánica, generación de fabricación reproducible.

Firmware: Compila sin warnings críticos, pruebas unitarias básicas, cobertura mínima acordada.

Verificación: Todos los tests de aceptación superados o mitigados con desviaciones aprobadas.

🔄 Control de cambios

Ramificación Git por fase (spec/, schematic/, pcb/, fw/, verification/).

Pull Requests con checklist de revisión y etiqueta de fase.

Versionado semántico de entregables (vX.Y.Z) y changelog por hito.

⚠️ Riesgos y mitigación

Disponibilidad de componentes: definir alternativos en BOM; compra temprana de críticos.

Retrabajo PCB: revisiones DRC/DFM/EMC cruzadas antes de generar gerbers.

Cuellos de botella de fabricación: doble ventana de envío/recepción (tareas 37–42).

Derivas térmicas/consumo: análisis en esquemático + validación en verificación.

Integración HW-FW: sprints de firmware en paralelo a fabricación para acortar time-to-bring-up.

📣 Comunicación y ceremonias

Reunión semanal de seguimiento (30–45 min): estado, bloqueos, próximos hitos.

Revisión de fase al cierre de cada hito (H1…H7) con acta y decisiones.

Gestión de incidencias en el tracker del repositorio (labels por fase).
