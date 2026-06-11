# Assets

Текущая версия игры использует внешние PNG-спрайты, sprite-sheet анимации, WAV-звуки и OGG-музыку.
Если часть ассетов не загрузится, игра использует процедурные fallback-эффекты и не должна падать при запуске.

Структура:

```text
assets/
├── textures/
│   └── sprites/
│       ├── player_ship.png
│       ├── asteroid_rock.png
│       ├── asteroid_fast.png
│       ├── asteroid_heavy.png
│       ├── pickup_score.png
│       ├── pickup_shield.png
│       ├── bullet.png
│       ├── enemy_projectile.png
│       ├── boss_cruiser.png
│       ├── boss_striker.png
│       ├── boss_carrier.png
│       ├── player_ship_anim.png
│       ├── boss_cruiser_anim.png
│       ├── boss_striker_anim.png
│       ├── boss_carrier_anim.png
│       └── explosion_anim.png
├── sounds/
│   ├── shot.wav
│   ├── pickup.wav
│   └── explosion.wav
├── music/
│   ├── menu_theme.ogg
│   ├── game_theme.ogg
│   ├── boss_theme.ogg
│   ├── gameover_theme.ogg
│   └── ATTRIBUTION.md
└── fonts/
```
