# MMCE Unofficial Rebuild – Änderungen

> Inoffizielle Version, nicht verbunden mit dem MMCE-Team oder CodeParade.

## Neue Funktionen (alles pro Level im Editor, F4 öffnet die Fenster)

**Level editor**
- Alle Zahlenwerte mit 7 Nachkommastellen (Positionen, Shift, Farben, Achsen …)
- *Sun and sky*: Sonnenrichtung (Pfeil + exakt), Sonnenfarbe, eigene Himmelsfarbe
  - Sonne unter dem Horizont (Y < 0) → komplett dunkel
  - Sonnenfarbe schwarz → keine Sonne, schwarzer Himmel
- *Game rules*: Jump + Sprunghöhe (Leertaste / Gamepad A), Rubber Ball (matt schwarz,
  Flummi-Physik, verformt sich beim Aufprall, wird beim Quetschen nicht zerstört),
  Bounciness, Ballgewicht, Kill Y an/aus, Lenk-Beschleunigung (Boden/Luft),
  Max. Geschwindigkeit, Max. Fallgeschwindigkeit, Reibung.
  Negative Schwerkraft → Ansicht dreht sich kopfüber (Kill Y wird gespiegelt: y > -kill_y)
- *Flags and checkpoints*: bis zu 16 zusätzliche Flaggen und 16 Checkpoints
  (Hinzufügen → auf das Fraktal klicken). *Multiple flag mode*: Flaggen müssen
  der Reihe nach erreicht werden, gesperrte Flaggen sind grau, erreichte grün,
  der Pfeil zeigt zur nächsten Flagge und trägt ihre Nummer. Ohne den Modus
  beendet jede Flagge das Level. Aktivierte Checkpoints leuchten grün; beim Tod
  respawnt der Ball dort (R / Rechtsklick startet das Level komplett neu).

**Fractal editor**
- Fractal Iterations ohne Obergrenze; Shader und Physik brechen ab, sobald weitere
  Iterationen nichts mehr ändern (max. ~40 echte Schleifen bei Scale 1.8)
- *Symmetry*: Typ (Würfel/Menger, Tetraeder/Sierpinski, Diagonale, nur Spiegel),
  Spiegelachsen X/Y/Z, radiale Symmetrie global und intern (Anzahl Achsen)
- *Fractal Animation*: Rotationsachse 1 und 2, „Reacts to music“ + Stärke
- *Fractal Glow*: eigene Glow-Farbe (unabhängig von der Oberflächenfarbe),
  Helligkeit, Glow-Muster, Beat-Sync + Stärke
- *Beat equalizer*: Empfindlichkeit für 5 Frequenzbänder + Live-Anzeige
  (Musik im Level editor mit „Play Music“ abspielen)

**Weitere Änderungen**
- Checkpoint-Fortschritt wird beim Aktivieren gespeichert (`levels/progress/<id>.cp`).
  Beim Start des Levels: Dialog „Start from new“ / „Start from checkpoint“.
  Ein vom Checkpoint fortgesetzter Lauf zählt nicht als Bestzeit.
- Multiple flag mode: Die Hauptflagge ist Flagge 1, danach folgen die Zusatzflaggen,
  die letzte beendet das Level.
- Tiefenschärfe skaliert mit der Ballgröße (keine Komplett-Unschärfe bei kleinen Bällen).
- Rubber Ball: eigener synthetischer „Boing“-Sound (Tonhöhe hängt von Gewicht/Aufprall ab).
- Einstellungen → Gameplay → *Crush effects*: Ein zerquetschter Glas-/Keramikball zerspringt
  in physikalische Scherben, ein Metallball wird zum Knödel; nach 3 s Respawn
  (Checkpoint oder Start).

**Spielregeln (Teil 2)**
- *Ordered checkpoints*: Checkpoints müssen der Reihe nach getroffen werden; der Pfeil zeigt
  „C1“, „C2“ …, der nächste Checkpoint leuchtet gelb, die Flaggen sind bis dahin gesperrt.
- Erreichte Flaggen (Multi-Flag) sind auch Respawn-Punkte.
- Zwischenzeiten bei jedem Checkpoint / jeder Flagge; die Gesamtzeit läuft nach dem Respawn weiter.
- *Time limit (s)*: Countdown oben rechts; läuft er ab, wird der Ball zerstört (auch der Flummi
  zerspringt) und das Level startet neu.
- R / Rechtsklick: zurück zum letzten Checkpoint bzw. zur letzten Flagge.
  Shift+R oder 2× R schnell: kompletter Neustart.

**Bugfixes**
- Kaputte .lvl-Dateien werden übersprungen statt als Müll-Level geladen.
- Absturz bei fehlender Level-Musik und bei leerer Musikliste im Editor behoben.
- Test-Shader noise.glsl kompiliert wieder; alte Fehler-Dump-Datei entfernt.

**Performance**
- Kein festes 60-fps-Limit mehr: Die Physik läuft weiter mit festen 60 Hz, gerendert wird so
  schnell wie *Framerate limit* (0 = unbegrenzt) / VSYNC erlauben; Kamera und Ball werden
  zwischen den Physik-Ticks interpoliert. VSYNC wirkt jetzt sofort beim Anwenden.
- Shader: Bounding-Sphere-Tests für Ball und Flagge, schneller Pfad für die Standard-Symmetrie.
- Uniform-Locations werden gecacht, glProgramUniform statt glUseProgram pro Uniform.

Alte .lvl-Dateien laden unverändert; gespeicherte Level hängen einen Erweiterungsblock an.

## Bauen

Siehe [README.md](../README.md#building-windows-msys2--clang): MSYS2 installieren, in der
CLANG64-Shell `./tools/setup_deps_msys2.sh`, danach `cmake` und `./tools/package_windows.sh`.
