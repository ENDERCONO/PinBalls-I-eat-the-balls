# PINBALL

Juego de pinball 2D hecho en C++ con **raylib** (gráficos, audio y entrada) y **Box2D** (físicas), para el Project 01: Classic Physics (CITM-UPC).

## Links

- **Código fuente (GitHub):** https://github.com/ENDERCONO/PinBalls-I-eat-the-balls
- **Release (zip del juego):** https://github.com/ENDERCONO/PinBalls-I-eat-the-balls/releases/tag/v1.0
- **Juego original:** no hemos copiado un juego concreto. El pinball (mesa, físicas, reglas y puntuación) está hecho desde cero por el grupo, inspirado en los pinballs 2D clásicos.

## Developers

- Jonathan Bermello
- Sandra Salvador
- Alejandro Pérez
- Ruixang Ji

## Description

Pinball 2D donde el objetivo es llegar a **1500 puntos** antes de perder las 3 bolas de la partida. Los flippers están unidos con revolute joints, el lanzador con un prismatic joint, y la mesa tiene bumpers, triángulos (slingshots) y una zona de ralentización.

## Controls

| Acción | Tecla |
|---|---|
| Flipper izquierdo | ← o A |
| Flipper derecho | → o D |
| Lanzar bola (kicker) | ↓ o W (mantener y soltar) |
| Menú / volver | Esc |
| Seleccionar en menús | Enter |
| Moverse por los menús | ↑ / ↓ |
| Ajustes de físicas | Tab (en partida) o desde el menú |
| Modo debug (formas y mouse joint) | F1 |

## Rules and scoring

- **Bumpers:** 100 puntos. Cada 3 golpes a bumpers aparece una **bola extra** (máximo 3 extra).
- **Triángulos (slingshots):** 50 puntos.
- **Paredes:** 10 puntos (con un pequeño cooldown).
- **Flippers, kicker y otros objetos:** 5 puntos.
- **Ganas** al llegar a 1500 puntos. **Pierdes** al quedarte sin bolas.
- El panel de la izquierda muestra la puntuación actual (SCORE), la anterior (LAST) y la más alta de la sesión (BEST).

## Physics Settings

Se abren con Tab o desde el menú:

- **TARGET FPS:** 30 a 120 (pasos de 15).
- **GRAVITY:** 1 a 30.
- **BUMPER KICK:** fuerza de rebote de los bumpers (5 a 40).
- **WALL BOUNCE:** coeficiente de rebote de las paredes (0 a 1.2).

## Differences with the original

No aplica: no hay un juego original del que copiemos el diseño.

## License

This project is licensed under an unmodified MIT license. Check [LICENSE](LICENSE) for further details.
