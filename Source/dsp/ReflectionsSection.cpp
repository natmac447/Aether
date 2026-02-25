#include "ReflectionsSection.h"
#include <cmath>

// =============================================================================
// 30 Room Geometry Shape Presets
// =============================================================================
// Organized: 0-9 Small, 10-19 Medium, 20-29 Large.
// Each shape produces a maximally different reflection pattern.
// Delay times in ms (base values at Room Size 1.0, scaled linearly for smaller).
// Gains normalized per channel. Pan: -1.0 = hard left, 1.0 = hard right.

const ShapePreset ReflectionsSection::kShapes[ReflectionsSection::kNumShapes] =
{
    // =========================================================================
    // SMALL ROOMS (0-9)
    // =========================================================================

    // 0: "The Parlour" -- Small symmetric room, regular even spacing
    // Rectangular room, ~4x4m. Short, evenly-spaced reflections.
    // Even tap spacing = regular flutter pattern, warm and intimate.
    {
        "The Parlour",
        "A modest, symmetric chamber of even proportion -- "
        "warm and intimate, with reflections arriving in orderly measure",
        // L taps: evenly spaced
        { 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.5f, 15.0f, 18.0f },
        { 0.85f, 0.72f, 0.60f, 0.50f, 0.40f, 0.32f, 0.25f, 0.18f },
        { -0.3f, 0.2f, -0.4f, 0.3f, -0.2f, 0.4f, -0.3f, 0.2f },
        // R taps: offset by ~1ms
        { 2.5f, 4.5f, 6.5f, 8.5f, 10.5f, 13.0f, 15.5f, 18.5f },
        { 0.82f, 0.70f, 0.58f, 0.48f, 0.38f, 0.30f, 0.23f, 0.16f },
        { 0.3f, -0.2f, 0.4f, -0.3f, 0.2f, -0.4f, 0.3f, -0.2f },
        0.15f,   // absorption: moderate
        0.3f,    // tail diffusion bias: low (regular, not dense)
        0.2f     // tail modal character: even modes
    },

    // 1: "The Alcove" -- Asymmetric, irregular angles
    // Non-rectangular room with alcoves and odd angles. Reflections
    // arrive in unpredictable clusters. Complex, lively character.
    {
        "The Alcove",
        "An irregular chamber of unexpected angles and hidden recesses -- "
        "sound wanders through asymmetric paths, alive with curious texture",
        // L taps: irregular spacing (asymmetric walls, alcoves)
        { 1.2f, 2.7f, 3.1f, 5.8f, 7.2f, 11.0f, 16.5f, 22.0f },
        { 0.75f, 0.68f, 0.80f, 0.45f, 0.55f, 0.35f, 0.28f, 0.15f },
        { -0.7f, 0.3f, -0.1f, 0.8f, -0.5f, 0.6f, -0.4f, 0.2f },
        // R taps: very different pattern (asymmetric room)
        { 1.8f, 2.2f, 4.5f, 6.0f, 8.5f, 12.5f, 17.0f, 23.5f },
        { 0.72f, 0.80f, 0.55f, 0.62f, 0.40f, 0.30f, 0.22f, 0.12f },
        { 0.5f, -0.6f, 0.8f, -0.2f, 0.4f, -0.7f, 0.3f, -0.5f },
        0.14f,
        0.55f,
        0.5f     // moderate mode clustering (irregular shapes scatter modes)
    },

    // 2: "The Crypt" -- Small, dense, stone-like
    // Low ceiling, irregular stone walls. Very dense early reflections
    // with rapid buildup. Dark character from stone absorption.
    {
        "The Crypt",
        "A low vault of ancient stone -- "
        "close walls conspire to multiply each sound into a dense, dark murmur",
        // L taps: very dense, short delays (low ceiling, close walls)
        { 0.8f, 1.5f, 2.0f, 2.8f, 3.5f, 4.8f, 6.5f, 9.0f },
        { 0.90f, 0.85f, 0.80f, 0.72f, 0.65f, 0.50f, 0.38f, 0.25f },
        { -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f },
        // R taps
        { 1.0f, 1.8f, 2.3f, 3.2f, 4.0f, 5.2f, 7.0f, 9.5f },
        { 0.88f, 0.82f, 0.77f, 0.70f, 0.62f, 0.48f, 0.35f, 0.22f },
        { 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f },
        0.22f,   // high absorption (rough stone)
        0.8f,    // very dense tail
        0.7f     // strong mode clustering
    },

    // 3: "The Vestibule" -- Small entryway, hard plaster walls, tile floor
    // ~2x3m entry hall. Quick, bright reflections from hard surfaces.
    {
        "The Vestibule",
        "A small entrance hall of hard plaster and tile -- "
        "crisp early reflections announce each arrival with clarity",
        { 1.0f, 2.2f, 3.5f, 4.8f, 6.0f, 7.5f, 9.5f, 12.0f },
        { 0.88f, 0.75f, 0.64f, 0.55f, 0.45f, 0.35f, 0.27f, 0.18f },
        { -0.3f, 0.4f, -0.2f, 0.3f, -0.4f, 0.2f, -0.3f, 0.4f },
        { 1.3f, 2.5f, 3.8f, 5.2f, 6.5f, 8.0f, 10.0f, 12.5f },
        { 0.85f, 0.72f, 0.62f, 0.52f, 0.43f, 0.33f, 0.25f, 0.16f },
        { 0.3f, -0.4f, 0.2f, -0.3f, 0.4f, -0.2f, 0.3f, -0.4f },
        0.10f,   // low absorption (hard plaster, tile)
        0.35f,
        0.25f
    },

    // 4: "The Closet" -- Very tiny enclosed space, fabric dampens
    // ~1.5x1.5m wardrobe. Extremely short delays, rapid decay from fabric.
    {
        "The Closet",
        "A narrow wardrobe of hanging cloth and soft wool -- "
        "sound arrives quickly and fades into plush silence",
        { 0.5f, 0.9f, 1.3f, 1.8f, 2.3f, 3.0f, 4.0f, 5.5f },
        { 0.92f, 0.85f, 0.78f, 0.68f, 0.58f, 0.45f, 0.32f, 0.20f },
        { -0.2f, 0.15f, -0.25f, 0.2f, -0.15f, 0.25f, -0.2f, 0.15f },
        { 0.6f, 1.1f, 1.5f, 2.0f, 2.5f, 3.3f, 4.3f, 5.8f },
        { 0.90f, 0.82f, 0.75f, 0.65f, 0.55f, 0.42f, 0.30f, 0.18f },
        { 0.2f, -0.15f, 0.25f, -0.2f, 0.15f, -0.25f, 0.2f, -0.15f },
        0.30f,   // high absorption (fabric, clothes)
        0.85f,
        0.8f
    },

    // 5: "The Study" -- Book-lined private room, warm absorption
    // ~3x4m with wooden desk and bookshelves. Warm, controlled character.
    {
        "The Study",
        "A scholar's retreat lined with leather-bound volumes -- "
        "warm reflections diffuse among the shelves in hushed tones",
        { 1.5f, 3.0f, 4.5f, 6.0f, 7.5f, 9.5f, 12.0f, 15.0f },
        { 0.78f, 0.65f, 0.55f, 0.46f, 0.38f, 0.30f, 0.22f, 0.15f },
        { -0.35f, 0.25f, -0.4f, 0.35f, -0.25f, 0.4f, -0.35f, 0.25f },
        { 1.8f, 3.3f, 4.8f, 6.5f, 8.0f, 10.0f, 12.5f, 15.5f },
        { 0.75f, 0.62f, 0.52f, 0.44f, 0.36f, 0.28f, 0.20f, 0.13f },
        { 0.35f, -0.25f, 0.4f, -0.35f, 0.25f, -0.4f, 0.35f, -0.25f },
        0.20f,   // moderate-high absorption (books, wood)
        0.45f,
        0.3f
    },

    // 6: "The Telephone Box" -- Extremely small, metallic, bright
    // ~0.8x0.8m glass and metal cabinet. Very short, ringing reflections.
    {
        "The Telephone Box",
        "A cramped metal-and-glass cabinet of startling resonance -- "
        "every sound multiplies in brilliant, ringing proximity",
        { 0.3f, 0.6f, 0.9f, 1.2f, 1.6f, 2.1f, 2.8f, 3.8f },
        { 0.95f, 0.90f, 0.85f, 0.78f, 0.70f, 0.60f, 0.48f, 0.35f },
        { -0.15f, 0.1f, -0.2f, 0.15f, -0.1f, 0.2f, -0.15f, 0.1f },
        { 0.4f, 0.7f, 1.0f, 1.4f, 1.8f, 2.3f, 3.0f, 4.0f },
        { 0.93f, 0.88f, 0.82f, 0.75f, 0.67f, 0.57f, 0.45f, 0.32f },
        { 0.15f, -0.1f, 0.2f, -0.15f, 0.1f, -0.2f, 0.15f, -0.1f },
        0.04f,   // very low absorption (glass, metal)
        0.9f,
        0.85f
    },

    // 7: "The Pantry" -- Small kitchen storage, shelved, complex scattering
    // ~2x2m with wooden shelves, jars, and crockery. Irregular scattering.
    {
        "The Pantry",
        "A cluttered larder of shelves and stoneware -- "
        "sound scatters among the jars and crockery in unexpected patterns",
        { 0.8f, 1.6f, 2.5f, 3.5f, 4.5f, 6.0f, 8.0f, 10.5f },
        { 0.82f, 0.73f, 0.65f, 0.55f, 0.48f, 0.38f, 0.28f, 0.18f },
        { -0.45f, 0.35f, -0.55f, 0.5f, -0.3f, 0.45f, -0.4f, 0.3f },
        { 1.0f, 1.9f, 2.8f, 3.8f, 5.0f, 6.5f, 8.5f, 11.0f },
        { 0.80f, 0.70f, 0.62f, 0.52f, 0.45f, 0.35f, 0.25f, 0.16f },
        { 0.5f, -0.4f, 0.6f, -0.45f, 0.35f, -0.5f, 0.4f, -0.35f },
        0.17f,
        0.6f,
        0.55f
    },

    // 8: "The Confessional" -- Narrow wooden booth, intimate
    // ~1x2m wood-panelled booth with curtain. Asymmetric, warm.
    {
        "The Confessional",
        "A narrow wooden enclosure of solemn privacy -- "
        "whispered sound lingers in the close, warm dark",
        { 0.6f, 1.2f, 1.8f, 2.5f, 3.5f, 5.0f, 7.0f, 10.0f },
        { 0.80f, 0.72f, 0.63f, 0.55f, 0.45f, 0.35f, 0.25f, 0.16f },
        { -0.5f, 0.2f, -0.6f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f },
        { 0.9f, 1.5f, 2.2f, 3.0f, 4.2f, 5.5f, 7.5f, 10.5f },
        { 0.78f, 0.68f, 0.60f, 0.50f, 0.42f, 0.32f, 0.22f, 0.14f },
        { 0.4f, -0.3f, 0.5f, -0.2f, 0.6f, -0.4f, 0.3f, -0.5f },
        0.25f,   // high absorption (wood panels, curtain)
        0.5f,
        0.6f
    },

    // 9: "The Powder Room" -- Small tiled bathroom, very bright
    // ~2x2.5m with ceramic tiles and mirror. Hard, bright reflections.
    {
        "The Powder Room",
        "A small chamber of porcelain and polished tile -- "
        "bright reflections cascade in a crystalline chorus",
        { 0.7f, 1.4f, 2.0f, 2.8f, 3.8f, 5.2f, 7.0f, 9.5f },
        { 0.90f, 0.82f, 0.75f, 0.67f, 0.58f, 0.45f, 0.33f, 0.22f },
        { -0.35f, 0.3f, -0.4f, 0.35f, -0.3f, 0.4f, -0.35f, 0.3f },
        { 0.9f, 1.6f, 2.3f, 3.2f, 4.2f, 5.5f, 7.5f, 10.0f },
        { 0.88f, 0.80f, 0.72f, 0.64f, 0.55f, 0.42f, 0.30f, 0.20f },
        { 0.35f, -0.3f, 0.4f, -0.35f, 0.3f, -0.4f, 0.35f, -0.3f },
        0.06f,   // very low absorption (ceramic, glass)
        0.7f,
        0.65f
    },

    // =========================================================================
    // MEDIUM ROOMS (10-19)
    // =========================================================================

    // 10: "The Chamber" -- Medium square, moderate density
    // Square room ~6x6m. Reflections from all four walls arrive at similar times,
    // creating a dense cluster. Classic studio live room feel.
    {
        "The Chamber",
        "A square room of generous dimension -- "
        "reflections converge from all quarters in a rich, even chorus",
        // L taps: clustered in the 3-8ms range (similar wall distances)
        { 3.0f, 3.8f, 4.5f, 5.2f, 6.0f, 7.5f, 9.5f, 13.0f },
        { 0.80f, 0.75f, 0.70f, 0.65f, 0.55f, 0.42f, 0.30f, 0.20f },
        { -0.5f, 0.4f, -0.3f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 3.3f, 4.2f, 5.0f, 5.7f, 6.5f, 8.0f, 10.0f, 13.5f },
        { 0.78f, 0.72f, 0.67f, 0.62f, 0.52f, 0.40f, 0.28f, 0.18f },
        { 0.5f, -0.4f, 0.3f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.18f,
        0.5f,    // moderate tail diffusion
        0.45f    // slight mode clustering (square room modes)
    },

    // 11: "The Gallery" -- Long rectangular, strong lateral reflections
    // Long narrow room, ~12x4m. First reflections from side walls are early;
    // end-wall reflections are late. Creates left-right width.
    {
        "The Gallery",
        "A long, narrow hall of distinguished proportion -- "
        "sound rebounds between close walls before reaching the distant end",
        // L taps: clustered early (side walls) then sparse late (end walls)
        { 1.5f, 2.8f, 3.5f, 5.0f, 9.0f, 14.0f, 20.0f, 27.0f },
        { 0.90f, 0.78f, 0.65f, 0.55f, 0.35f, 0.25f, 0.18f, 0.12f },
        { -0.8f, 0.7f, -0.6f, 0.5f, -0.3f, 0.2f, -0.1f, 0.05f },
        // R taps
        { 1.8f, 3.2f, 4.0f, 5.5f, 9.5f, 14.5f, 20.5f, 27.5f },
        { 0.88f, 0.75f, 0.62f, 0.52f, 0.33f, 0.23f, 0.16f, 0.10f },
        { 0.8f, -0.7f, 0.6f, -0.5f, 0.3f, -0.2f, 0.1f, -0.05f },
        0.12f,
        0.4f,
        0.35f
    },

    // 12: "The Conservatory" -- Bright, glass-like, wide
    // Large room with hard, reflective surfaces. Bright, widely spread.
    {
        "The Conservatory",
        "A luminous glass pavilion of generous span -- "
        "bright reflections dance between crystalline surfaces with brilliant clarity",
        // L taps: wide spread, moderate density
        { 2.0f, 4.5f, 7.0f, 10.0f, 14.0f, 18.5f, 23.0f, 28.0f },
        { 0.88f, 0.75f, 0.65f, 0.55f, 0.45f, 0.35f, 0.25f, 0.18f },
        { -0.9f, 0.8f, -0.7f, 0.6f, -0.5f, 0.7f, -0.6f, 0.5f },
        // R taps
        { 2.5f, 5.0f, 7.5f, 10.5f, 14.5f, 19.0f, 23.5f, 28.5f },
        { 0.85f, 0.72f, 0.62f, 0.52f, 0.42f, 0.32f, 0.23f, 0.15f },
        { 0.9f, -0.8f, 0.7f, -0.6f, 0.5f, -0.7f, 0.6f, -0.5f },
        0.05f,   // very low absorption (glass/hard surfaces)
        0.35f,
        0.15f    // even modes (regular geometry)
    },

    // 13: "The Scriptorium" -- Stone writing room, vaulted ceiling
    // ~5x7m monastery room. Stone walls, arched ceiling, wooden desks.
    {
        "The Scriptorium",
        "A vaulted stone chamber where monks once copied manuscripts -- "
        "rich, dark tones linger beneath the arched ceiling",
        { 2.5f, 4.0f, 6.0f, 8.0f, 10.5f, 13.5f, 17.0f, 21.0f },
        { 0.82f, 0.72f, 0.62f, 0.53f, 0.44f, 0.35f, 0.26f, 0.18f },
        { -0.5f, 0.4f, -0.6f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        { 3.0f, 4.5f, 6.5f, 8.5f, 11.0f, 14.0f, 17.5f, 21.5f },
        { 0.80f, 0.70f, 0.60f, 0.50f, 0.42f, 0.33f, 0.24f, 0.16f },
        { 0.5f, -0.4f, 0.6f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.10f,   // low absorption (stone)
        0.55f,
        0.4f
    },

    // 14: "The Library" -- Medium room lined with books, carpet
    // ~6x8m with towering shelves. Very absorptive, warm, dead-ish.
    {
        "The Library",
        "A hushed hall of towering bookshelves and deep carpet -- "
        "sound is swallowed quickly into the embrace of a thousand volumes",
        { 2.0f, 4.5f, 7.0f, 9.5f, 12.0f, 15.0f, 19.0f, 23.0f },
        { 0.72f, 0.60f, 0.50f, 0.42f, 0.34f, 0.26f, 0.18f, 0.12f },
        { -0.4f, 0.3f, -0.5f, 0.4f, -0.35f, 0.45f, -0.4f, 0.3f },
        { 2.5f, 5.0f, 7.5f, 10.0f, 12.5f, 15.5f, 19.5f, 23.5f },
        { 0.70f, 0.58f, 0.48f, 0.40f, 0.32f, 0.24f, 0.16f, 0.10f },
        { 0.4f, -0.3f, 0.5f, -0.4f, 0.35f, -0.45f, 0.4f, -0.3f },
        0.24f,   // high absorption (books, carpet)
        0.3f,
        0.2f
    },

    // 15: "The Drawing Room" -- Elegant Victorian medium room
    // ~5x6m with plaster walls, rugs, upholstered furniture.
    {
        "The Drawing Room",
        "An elegant parlour of velvet and polished wood -- "
        "refined reflections arrive with measured grace",
        { 1.8f, 3.5f, 5.5f, 7.5f, 10.0f, 13.0f, 16.5f, 20.0f },
        { 0.80f, 0.68f, 0.58f, 0.50f, 0.42f, 0.33f, 0.24f, 0.16f },
        { -0.45f, 0.35f, -0.5f, 0.4f, -0.35f, 0.5f, -0.4f, 0.35f },
        { 2.2f, 4.0f, 6.0f, 8.0f, 10.5f, 13.5f, 17.0f, 20.5f },
        { 0.78f, 0.65f, 0.55f, 0.47f, 0.40f, 0.31f, 0.22f, 0.14f },
        { 0.45f, -0.35f, 0.5f, -0.4f, 0.35f, -0.5f, 0.4f, -0.35f },
        0.16f,   // moderate absorption (plaster + furnishings)
        0.4f,
        0.3f
    },

    // 16: "The Workshop" -- Industrial medium space, hard surfaces
    // ~6x8m concrete floor, metal equipment, high ceiling.
    {
        "The Workshop",
        "A workman's hall of concrete and bare steel -- "
        "hammered sound rings bright and hard among the tools",
        { 2.0f, 3.5f, 5.0f, 7.5f, 10.0f, 14.0f, 18.0f, 23.0f },
        { 0.85f, 0.73f, 0.63f, 0.52f, 0.42f, 0.32f, 0.22f, 0.14f },
        { -0.6f, 0.5f, -0.7f, 0.6f, -0.5f, 0.4f, -0.6f, 0.5f },
        { 2.3f, 4.0f, 5.8f, 8.0f, 11.0f, 15.0f, 19.0f, 24.0f },
        { 0.82f, 0.70f, 0.60f, 0.50f, 0.40f, 0.30f, 0.20f, 0.12f },
        { 0.6f, -0.5f, 0.7f, -0.6f, 0.5f, -0.4f, 0.6f, -0.5f },
        0.08f,   // low absorption (concrete, metal)
        0.5f,
        0.45f
    },

    // 17: "The Refectory" -- Monastic dining hall, stone and wood
    // ~7x12m with stone walls, long wooden tables, high ceiling.
    {
        "The Refectory",
        "A long monastic dining hall of cold stone and worn oak -- "
        "voices travel the length of the table before returning from the far wall",
        { 2.5f, 5.0f, 7.5f, 10.5f, 14.0f, 18.0f, 22.0f, 26.0f },
        { 0.82f, 0.70f, 0.60f, 0.50f, 0.40f, 0.30f, 0.22f, 0.14f },
        { -0.7f, 0.5f, -0.6f, 0.7f, -0.4f, 0.3f, -0.5f, 0.4f },
        { 3.0f, 5.5f, 8.0f, 11.0f, 14.5f, 18.5f, 22.5f, 26.5f },
        { 0.80f, 0.68f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.12f },
        { 0.7f, -0.5f, 0.6f, -0.7f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.13f,   // moderate absorption (stone + wood)
        0.5f,
        0.4f
    },

    // 18: "The Solarium" -- Sunroom with glass walls, plants
    // ~5x5m glass walls, wrought iron, some plants and wicker.
    {
        "The Solarium",
        "A sunlit room of glass and wrought iron -- "
        "light and sound mingle in an airy, luminous space",
        { 1.5f, 3.2f, 5.0f, 7.0f, 9.5f, 12.5f, 16.0f, 20.0f },
        { 0.85f, 0.74f, 0.64f, 0.55f, 0.46f, 0.36f, 0.27f, 0.18f },
        { -0.6f, 0.5f, -0.55f, 0.45f, -0.5f, 0.55f, -0.45f, 0.4f },
        { 2.0f, 3.7f, 5.5f, 7.5f, 10.0f, 13.0f, 16.5f, 20.5f },
        { 0.82f, 0.72f, 0.62f, 0.52f, 0.44f, 0.34f, 0.25f, 0.16f },
        { 0.6f, -0.5f, 0.55f, -0.45f, 0.5f, -0.55f, 0.45f, -0.4f },
        0.09f,   // low absorption (glass dominant)
        0.4f,
        0.25f
    },

    // 19: "The Apothecary" -- Wooden cabinets, glass bottles, complex scattering
    // ~4x6m with many small objects creating diffuse reflections.
    {
        "The Apothecary",
        "A cabinet of curiosities lined with bottles and strange wares -- "
        "each surface scatters sound in intricate, unpredictable ways",
        { 1.8f, 3.0f, 4.5f, 6.5f, 8.5f, 11.5f, 15.0f, 19.0f },
        { 0.78f, 0.72f, 0.65f, 0.55f, 0.48f, 0.38f, 0.28f, 0.18f },
        { -0.55f, 0.65f, -0.4f, 0.7f, -0.6f, 0.35f, -0.5f, 0.6f },
        { 2.2f, 3.5f, 5.0f, 7.0f, 9.0f, 12.0f, 15.5f, 19.5f },
        { 0.75f, 0.70f, 0.62f, 0.52f, 0.45f, 0.35f, 0.25f, 0.15f },
        { 0.6f, -0.5f, 0.45f, -0.65f, 0.55f, -0.4f, 0.7f, -0.5f },
        0.14f,   // moderate absorption (wood + glass)
        0.65f,
        0.5f
    },

    // =========================================================================
    // LARGE ROOMS (20-29)
    // =========================================================================

    // 20: "The Nave" -- Large cathedral-like, very long reverb path
    // Tall, long space ~20x8m with high ceiling. Reflections from
    // floor/ceiling arrive first, wall reflections spread across time.
    {
        "The Nave",
        "A vast vaulted space of soaring height -- "
        "sound ascends to the ceiling before cascading down in waves of grandeur",
        // L taps: floor/ceiling early, walls spread late
        { 1.0f, 2.5f, 5.0f, 8.5f, 13.0f, 18.0f, 24.0f, 30.0f },
        { 0.70f, 0.82f, 0.60f, 0.50f, 0.40f, 0.30f, 0.22f, 0.15f },
        { -0.2f, 0.1f, -0.6f, 0.5f, -0.4f, 0.3f, -0.5f, 0.4f },
        // R taps
        { 1.3f, 3.0f, 5.5f, 9.0f, 13.5f, 18.5f, 24.5f, 30.0f },
        { 0.68f, 0.80f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.13f },
        { 0.2f, -0.1f, 0.6f, -0.5f, 0.4f, -0.3f, 0.5f, -0.4f },
        0.08f,   // low absorption (stone/plaster surfaces)
        0.7f,    // high tail diffusion
        0.6f     // clustered modes (resonant space)
    },

    // 21: "The Ballroom" -- Grand dancing hall, polished floors
    // ~15x20m with high ceiling, polished wood floor, plaster walls.
    {
        "The Ballroom",
        "A grand hall of polished parquet and gilded mouldings -- "
        "sound sweeps across the dance floor in broad, elegant arcs",
        { 3.0f, 6.0f, 9.0f, 13.0f, 17.0f, 22.0f, 27.0f, 30.0f },
        { 0.78f, 0.67f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.14f },
        { -0.8f, 0.7f, -0.65f, 0.75f, -0.6f, 0.5f, -0.7f, 0.6f },
        { 3.5f, 6.5f, 9.5f, 13.5f, 17.5f, 22.5f, 27.5f, 30.0f },
        { 0.75f, 0.64f, 0.55f, 0.45f, 0.36f, 0.26f, 0.18f, 0.12f },
        { 0.8f, -0.7f, 0.65f, -0.75f, 0.6f, -0.5f, 0.7f, -0.6f },
        0.07f,   // low absorption (polished surfaces)
        0.45f,
        0.3f
    },

    // 22: "The Atrium" -- Open central courtyard with columns
    // ~12x12m open courtyard, stone columns, open above.
    {
        "The Atrium",
        "An open courtyard framed by stone columns and sky -- "
        "reflections scatter among the pillars while the ceiling is infinite",
        { 2.0f, 4.5f, 7.0f, 10.0f, 14.0f, 19.0f, 25.0f, 30.0f },
        { 0.75f, 0.65f, 0.55f, 0.48f, 0.38f, 0.28f, 0.20f, 0.13f },
        { -0.7f, 0.6f, -0.8f, 0.5f, -0.6f, 0.7f, -0.5f, 0.4f },
        { 2.5f, 5.0f, 7.5f, 10.5f, 14.5f, 19.5f, 25.5f, 30.0f },
        { 0.72f, 0.62f, 0.52f, 0.45f, 0.36f, 0.26f, 0.18f, 0.11f },
        { 0.7f, -0.6f, 0.8f, -0.5f, 0.6f, -0.7f, 0.5f, -0.4f },
        0.11f,   // moderate absorption (stone, open ceiling loses energy)
        0.4f,
        0.35f
    },

    // 23: "The Chapel" -- Small church, pointed arches
    // ~8x15m with pointed arch ceiling, stone walls, wood pews.
    {
        "The Chapel",
        "A sacred space of pointed arches and stained glass -- "
        "prayer and song rise toward heaven in cascading waves of devotion",
        { 1.5f, 3.5f, 6.0f, 9.0f, 13.0f, 17.5f, 23.0f, 29.0f },
        { 0.76f, 0.68f, 0.58f, 0.48f, 0.38f, 0.28f, 0.20f, 0.14f },
        { -0.4f, 0.3f, -0.6f, 0.5f, -0.45f, 0.55f, -0.5f, 0.4f },
        { 2.0f, 4.0f, 6.5f, 9.5f, 13.5f, 18.0f, 23.5f, 29.5f },
        { 0.74f, 0.65f, 0.55f, 0.45f, 0.36f, 0.26f, 0.18f, 0.12f },
        { 0.4f, -0.3f, 0.6f, -0.5f, 0.45f, -0.55f, 0.5f, -0.4f },
        0.09f,   // low absorption (stone + stained glass)
        0.65f,
        0.55f
    },

    // 24: "The Warehouse" -- Large industrial, concrete and steel
    // Very large ~20x30m, concrete walls, steel beams, high ceiling.
    {
        "The Warehouse",
        "A cavernous industrial space of bare concrete and steel beams -- "
        "harsh echoes ricochet between the vast parallel walls",
        { 3.0f, 5.5f, 8.5f, 12.0f, 16.0f, 21.0f, 26.0f, 30.0f },
        { 0.82f, 0.70f, 0.60f, 0.50f, 0.40f, 0.30f, 0.22f, 0.15f },
        { -0.85f, 0.8f, -0.75f, 0.7f, -0.6f, 0.5f, -0.65f, 0.55f },
        { 3.8f, 6.5f, 9.5f, 13.0f, 17.0f, 22.0f, 27.0f, 30.0f },
        { 0.80f, 0.68f, 0.57f, 0.47f, 0.38f, 0.28f, 0.20f, 0.13f },
        { 0.85f, -0.8f, 0.75f, -0.7f, 0.6f, -0.5f, 0.65f, -0.55f },
        0.04f,   // very low absorption (concrete, metal)
        0.35f,
        0.4f
    },

    // 25: "The Cistern" -- Underground water reservoir, extremely reverberant
    // ~10x10m underground concrete with high dome, water surface.
    {
        "The Cistern",
        "A subterranean water chamber of vast concrete curves -- "
        "sound pours endlessly through the resonant dark, refusing to die",
        { 1.0f, 3.0f, 6.0f, 10.0f, 15.0f, 20.0f, 26.0f, 30.0f },
        { 0.72f, 0.68f, 0.62f, 0.55f, 0.48f, 0.40f, 0.32f, 0.25f },
        { -0.5f, 0.4f, -0.6f, 0.5f, -0.4f, 0.6f, -0.5f, 0.3f },
        { 1.5f, 3.5f, 6.5f, 10.5f, 15.5f, 20.5f, 26.5f, 30.0f },
        { 0.70f, 0.65f, 0.60f, 0.52f, 0.45f, 0.38f, 0.30f, 0.22f },
        { 0.5f, -0.4f, 0.6f, -0.5f, 0.4f, -0.6f, 0.5f, -0.3f },
        0.03f,   // extremely low absorption (concrete, water)
        0.85f,
        0.75f
    },

    // 26: "The Observatory" -- Domed room with curved ceiling
    // ~10m diameter dome. Curved surfaces focus reflections.
    {
        "The Observatory",
        "A domed chamber where curved walls focus sound to a point -- "
        "whispers converge from unexpected directions beneath the great lens",
        { 2.0f, 3.5f, 5.5f, 8.0f, 12.0f, 17.0f, 23.0f, 30.0f },
        { 0.70f, 0.75f, 0.65f, 0.55f, 0.45f, 0.35f, 0.25f, 0.16f },
        { -0.3f, 0.5f, -0.7f, 0.2f, -0.6f, 0.4f, -0.5f, 0.6f },
        { 2.5f, 4.0f, 6.0f, 8.5f, 12.5f, 17.5f, 23.5f, 30.0f },
        { 0.68f, 0.73f, 0.62f, 0.52f, 0.42f, 0.33f, 0.23f, 0.14f },
        { 0.3f, -0.5f, 0.7f, -0.2f, 0.6f, -0.4f, 0.5f, -0.6f },
        0.07f,   // low absorption (plaster dome)
        0.6f,
        0.65f    // dome focuses modes, creating clusters
    },

    // 27: "The Great Hall" -- Medieval feasting hall, stone and timber
    // ~12x25m with high timber roof, stone walls.
    {
        "The Great Hall",
        "A vast medieval hall of stone walls and soaring timber roof -- "
        "the warmth of wood and the chill of stone meet in majestic resonance",
        { 2.5f, 5.0f, 8.0f, 11.5f, 15.5f, 20.0f, 25.0f, 30.0f },
        { 0.76f, 0.65f, 0.56f, 0.47f, 0.38f, 0.30f, 0.22f, 0.15f },
        { -0.7f, 0.6f, -0.5f, 0.7f, -0.6f, 0.4f, -0.5f, 0.6f },
        { 3.0f, 5.5f, 8.5f, 12.0f, 16.0f, 20.5f, 25.5f, 30.0f },
        { 0.73f, 0.62f, 0.53f, 0.44f, 0.36f, 0.28f, 0.20f, 0.13f },
        { 0.7f, -0.6f, 0.5f, -0.7f, 0.6f, -0.4f, 0.5f, -0.6f },
        0.11f,   // moderate absorption (stone + timber)
        0.55f,
        0.45f
    },

    // 28: "The Greenhouse" -- Very large glass structure
    // ~10x15m all glass and iron frame. Extremely bright.
    {
        "The Greenhouse",
        "A vast glass cathedral of iron and light -- "
        "every sound rings bright and clear through the crystalline vault",
        { 2.5f, 5.5f, 8.5f, 12.0f, 16.0f, 20.5f, 25.0f, 30.0f },
        { 0.85f, 0.74f, 0.64f, 0.54f, 0.44f, 0.34f, 0.25f, 0.17f },
        { -0.85f, 0.75f, -0.7f, 0.8f, -0.65f, 0.6f, -0.75f, 0.7f },
        { 3.0f, 6.0f, 9.0f, 12.5f, 16.5f, 21.0f, 25.5f, 30.0f },
        { 0.82f, 0.72f, 0.62f, 0.52f, 0.42f, 0.32f, 0.23f, 0.15f },
        { 0.85f, -0.75f, 0.7f, -0.8f, 0.65f, -0.6f, 0.75f, -0.7f },
        0.03f,   // extremely low absorption (glass, metal)
        0.3f,
        0.2f
    },

    // 29: "The Mausoleum" -- Grand stone memorial, marble surfaces
    // ~8x8m with high dome, all marble and polished stone.
    {
        "The Mausoleum",
        "A solemn monument of polished marble and cold stone -- "
        "every whisper returns as a haunting echo from the vaulted silence",
        { 1.5f, 3.5f, 6.5f, 10.0f, 14.5f, 19.0f, 24.5f, 30.0f },
        { 0.74f, 0.68f, 0.60f, 0.52f, 0.43f, 0.34f, 0.25f, 0.17f },
        { -0.4f, 0.5f, -0.55f, 0.45f, -0.5f, 0.55f, -0.45f, 0.4f },
        { 2.0f, 4.0f, 7.0f, 10.5f, 15.0f, 19.5f, 25.0f, 30.0f },
        { 0.72f, 0.65f, 0.57f, 0.50f, 0.40f, 0.32f, 0.23f, 0.15f },
        { 0.4f, -0.5f, 0.55f, -0.45f, 0.5f, -0.55f, 0.45f, -0.4f },
        0.05f,   // very low absorption (marble, stone)
        0.7f,
        0.6f
    }
};

// =============================================================================
// roomSizeToDelayMs -- Maps normalized 0-1 knob value to delay time in ms
// =============================================================================
// Room Size 0.0 -> 1ms base delay, Room Size 1.0 -> 30ms base delay.
// The perceptual weighting is already in the NormalisableRange skew (0.4).

float ReflectionsSection::roomSizeToDelayMs (float normalized)
{
    return 1.0f + normalized * 29.0f;  // Linear 1-30ms
}

// =============================================================================
// prepare
// =============================================================================

void ReflectionsSection::prepare (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Size delay lines for max 35ms (30ms + margin)
    int maxSamples = static_cast<int> (std::ceil (35.0 * sampleRate / 1000.0));

    delayLineL.resize (maxSamples + 1);
    delayLineL.reset();
    delayLineR.resize (maxSamples + 1);
    delayLineR.reset();

    // Reset all 16 tap filters
    for (int t = 0; t < kTapsPerChannel; ++t)
    {
        tapFiltersL[t].reset();
        tapFiltersR[t].reset();
    }

    // Initialize SmoothedValues
    roomSizeSmoothed.reset (sampleRate, 0.050);    // 50ms ramp for delay changes
    proximitySmoothed.reset (sampleRate, 0.020);    // 20ms ramp
    widthSmoothed.reset (sampleRate, 0.020);         // 20ms ramp
    bypassBlend.reset (sampleRate, 0.010);           // 10ms bypass crossfade

    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    // Initialize shape crossfade state
    pendingShapeIndex = -1;
    shapeCrossfade = 1.0f;
    // 30ms crossfade: step per sample
    shapeCrossfadeStep = static_cast<float> (1.0 / (0.030 * sampleRate));
}

// =============================================================================
// process -- Per-sample stereo tapped delay line processing
// =============================================================================

void ReflectionsSection::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels < 1 || numSamples < 1)
        return;

    // Early exit: if fully bypassed and not smoothing, skip
    if (! bypassBlend.isSmoothing() && bypassBlend.getTargetValue() <= 0.0f)
        return;

    float* channelL = buffer.getWritePointer (0);
    float* channelR = (numChannels >= 2) ? buffer.getWritePointer (1) : nullptr;

    const float sr = static_cast<float> (currentSampleRate);
    const float minDelaySamples = sr * 0.001f;  // 1ms floor at any sample rate

    // Precompute average gain sums per shape for coherence normalization.
    // Computed once (static) since kShapes is constant.
    static float sGainSums[kNumShapes] = {};
    static bool sGainSumsReady = false;
    if (! sGainSumsReady)
    {
        for (int si = 0; si < kNumShapes; ++si)
            for (int t = 0; t < kTapsPerChannel; ++t)
                sGainSums[si] += (kShapes[si].gainL[t] + kShapes[si].gainR[t]) * 0.5f;
        sGainSumsReady = true;
    }

    for (int s = 0; s < numSamples; ++s)
    {
        // a. Read smoothed parameter values
        float roomSize = roomSizeSmoothed.getNextValue();
        float proximity = proximitySmoothed.getNextValue();
        float widthNorm = widthSmoothed.getNextValue();
        float blend = bypassBlend.getNextValue();

        // b. Compute delay multiplier from Room Size
        float delayMs = roomSizeToDelayMs (roomSize);

        // Subtle room size darkening: larger rooms get darker absorption filters
        // baseCutoff = 12kHz at min size, 6kHz at max size
        // Cross-stage: Air darkening further reduces cutoff (up to 3600Hz at max)
        float airCutoffReduction = airDarkeningFactor * 3000.0f;
        float baseCutoff = 12000.0f - roomSize * 6000.0f - airCutoffReduction;
        baseCutoff = juce::jmax (2000.0f, baseCutoff);

        // c. Get input samples
        float inputL = channelL[s];
        float inputR = (channelR != nullptr) ? channelR[s] : inputL;

        // d. Write input samples to delay buffers
        delayLineL.write (inputL);
        delayLineR.write (inputR);

        // e. Accumulate taps from shapes
        float sumL = 0.0f;
        float sumR = 0.0f;

        // Shape crossfade: determine blend gains
        float currentGain = 1.0f;
        float pendingGain = 0.0f;

        if (pendingShapeIndex >= 0)
        {
            // Shape crossfade in progress
            shapeCrossfade -= shapeCrossfadeStep;
            if (shapeCrossfade <= 0.0f)
            {
                // Crossfade complete
                currentShapeIndex = pendingShapeIndex;
                pendingShapeIndex = -1;
                shapeCrossfade = 1.0f;
                currentGain = 1.0f;
                pendingGain = 0.0f;
            }
            else
            {
                currentGain = shapeCrossfade;
                pendingGain = 1.0f - shapeCrossfade;
            }
        }

        // Accumulate taps from current shape
        {
            const ShapePreset& sh = kShapes[currentShapeIndex];
            for (int t = 0; t < kTapsPerChannel; ++t)
            {
                // Width interpolation for delay times
                float monoDelay = (sh.delayMsL[t] + sh.delayMsR[t]) * 0.5f;
                float lDMs = monoDelay + widthNorm * (sh.delayMsL[t] - monoDelay);
                float rDMs = monoDelay + widthNorm * (sh.delayMsR[t] - monoDelay);

                // Scale by room size
                lDMs = lDMs * delayMs / 30.0f;
                rDMs = rDMs * delayMs / 30.0f;

                float lSamp = juce::jmax (minDelaySamples, lDMs * sr / 1000.0f);
                float rSamp = juce::jmax (minDelaySamples, rDMs * sr / 1000.0f);

                float tapL = delayLineL.read (lSamp);
                float tapR = delayLineR.read (rSamp);

                // Per-tap absorption filter
                float cutL = juce::jlimit (1000.0f, 16000.0f,
                    baseCutoff - sh.absorptionRate * lDMs * 200.0f);
                float cutR = juce::jlimit (1000.0f, 16000.0f,
                    baseCutoff - sh.absorptionRate * rDMs * 200.0f);

                // Only update filter coefficients for current shape (not during crossfade)
                if (pendingShapeIndex < 0)
                {
                    tapFiltersL[t].lowpass (cutL / sr);
                    tapFiltersR[t].lowpass (cutR / sr);
                }

                tapL = tapFiltersL[t] (tapL);
                tapR = tapFiltersR[t] (tapR);

                // Width interpolation for gains
                float monoGain = (sh.gainL[t] + sh.gainR[t]) * 0.5f;
                float gL = monoGain + widthNorm * (sh.gainL[t] - monoGain);
                float gR = monoGain + widthNorm * (sh.gainR[t] - monoGain);
                tapL *= gL;
                tapR *= gR;

                // Width interpolation for pan
                float monoPan = (sh.panL[t] + sh.panR[t]) * 0.5f;
                float pL = monoPan + widthNorm * (sh.panL[t] - monoPan);
                float pR = monoPan + widthNorm * (sh.panR[t] - monoPan);

                // Linear pan law
                float pLL = 0.5f * (1.0f - pL);
                float pLR = 0.5f * (1.0f + pL);
                float pRL = 0.5f * (1.0f - pR);
                float pRR = 0.5f * (1.0f + pR);

                sumL += (tapL * pLL + tapR * pRL) * currentGain;
                sumR += (tapL * pLR + tapR * pRR) * currentGain;
            }
        }

        // Accumulate taps from pending shape (during crossfade)
        if (pendingShapeIndex >= 0 && pendingGain > 0.0f)
        {
            const ShapePreset& sh = kShapes[pendingShapeIndex];
            for (int t = 0; t < kTapsPerChannel; ++t)
            {
                float monoDelay = (sh.delayMsL[t] + sh.delayMsR[t]) * 0.5f;
                float lDMs = monoDelay + widthNorm * (sh.delayMsL[t] - monoDelay);
                float rDMs = monoDelay + widthNorm * (sh.delayMsR[t] - monoDelay);

                lDMs = lDMs * delayMs / 30.0f;
                rDMs = rDMs * delayMs / 30.0f;

                float lSamp = juce::jmax (minDelaySamples, lDMs * sr / 1000.0f);
                float rSamp = juce::jmax (minDelaySamples, rDMs * sr / 1000.0f);

                float tapL = delayLineL.read (lSamp);
                float tapR = delayLineR.read (rSamp);

                // Use unfiltered taps during crossfade to avoid filter coefficient fights
                // The crossfade is short (30ms) so this is inaudible

                float monoGain = (sh.gainL[t] + sh.gainR[t]) * 0.5f;
                float gL = monoGain + widthNorm * (sh.gainL[t] - monoGain);
                float gR = monoGain + widthNorm * (sh.gainR[t] - monoGain);
                tapL *= gL;
                tapR *= gR;

                float monoPan = (sh.panL[t] + sh.panR[t]) * 0.5f;
                float pL = monoPan + widthNorm * (sh.panL[t] - monoPan);
                float pR = monoPan + widthNorm * (sh.panR[t] - monoPan);

                float pLL = 0.5f * (1.0f - pL);
                float pLR = 0.5f * (1.0f + pL);
                float pRL = 0.5f * (1.0f - pR);
                float pRR = 0.5f * (1.0f + pR);

                sumL += (tapL * pLL + tapR * pRL) * pendingGain;
                sumR += (tapL * pLR + tapR * pRR) * pendingGain;
            }
        }

        // f. Compensate for coherent tap summation at small room sizes.
        // When room size is small, all 8 taps collapse to the 1ms delay floor
        // and read the same sample, causing gain sums of 3.6-5.0x (+11 to +14dB).
        // Normalize by room-size-dependent coherence: full at min, none at max.
        {
            float activeGS = sGainSums[currentShapeIndex];
            if (pendingShapeIndex >= 0)
                activeGS = currentGain * sGainSums[currentShapeIndex]
                         + pendingGain * sGainSums[pendingShapeIndex];

            float coherence = (1.0f - roomSize) * (1.0f - roomSize);
            float normDivisor = 1.0f + coherence * (activeGS - 1.0f);
            sumL /= normDivisor;
            sumR /= normDivisor;
        }

        // Apply proximity blend
        // Near (0.0): direct at 0dB, reflected at -18dB
        // Far (1.0): direct at -12dB, reflected at 0dB
        static constexpr float kMinus12dB = 0.251189f; // pow(10, -12/20)
        static constexpr float kMinus18dB = 0.125893f; // pow(10, -18/20)
        float directGain = 1.0f - proximity * (1.0f - kMinus12dB);
        float reflGain = kMinus18dB + proximity * (1.0f - kMinus18dB);

        float outputL = inputL * directGain + sumL * reflGain;
        float outputR = inputR * directGain + sumR * reflGain;

        // Apply bypass crossfade
        float finalL = inputL * (1.0f - blend) + outputL * blend;
        float finalR = inputR * (1.0f - blend) + outputR * blend;

        // Write output
        channelL[s] = finalL;
        if (channelR != nullptr)
            channelR[s] = finalR;
    }
}

// =============================================================================
// setRoomSize
// =============================================================================

void ReflectionsSection::setRoomSize (float size)
{
    roomSizeSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, size));
}

// =============================================================================
// setShape -- triggers 30ms crossfade to new shape
// =============================================================================

void ReflectionsSection::setShape (int shapeIndex)
{
    shapeIndex = juce::jlimit (0, kNumShapes - 1, shapeIndex);

    // Only trigger crossfade if actually changing
    if (shapeIndex != currentShapeIndex && shapeIndex != pendingShapeIndex)
    {
        pendingShapeIndex = shapeIndex;
        shapeCrossfade = 1.0f;
    }
}

// =============================================================================
// setProximity
// =============================================================================

void ReflectionsSection::setProximity (float proximity)
{
    proximitySmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, proximity));
}

// =============================================================================
// setWidth
// =============================================================================

void ReflectionsSection::setWidth (float width)
{
    widthSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, width));
}

// =============================================================================
// setAirDarkening -- Cross-stage coupling from Air Amount * Character scale
// =============================================================================

void ReflectionsSection::setAirDarkening (float darkening)
{
    airDarkeningFactor = juce::jlimit (0.0f, 2.0f, darkening);
}

// =============================================================================
// setBypass
// =============================================================================

void ReflectionsSection::setBypass (bool shouldBypass)
{
    bypassed = shouldBypass;
    bypassBlend.setTargetValue (shouldBypass ? 0.0f : 1.0f);
}

// =============================================================================
// getShapePreset
// =============================================================================

const ShapePreset& ReflectionsSection::getShapePreset (int index) const
{
    return kShapes[juce::jlimit (0, kNumShapes - 1, index)];
}

// =============================================================================
// reset
// =============================================================================

void ReflectionsSection::reset()
{
    delayLineL.reset();
    delayLineR.reset();

    for (int t = 0; t < kTapsPerChannel; ++t)
    {
        tapFiltersL[t].reset();
        tapFiltersR[t].reset();
    }

    roomSizeSmoothed.setCurrentAndTargetValue (0.4f);
    proximitySmoothed.setCurrentAndTargetValue (0.3f);
    widthSmoothed.setCurrentAndTargetValue (0.7f);
    bypassBlend.setCurrentAndTargetValue (bypassed ? 0.0f : 1.0f);

    pendingShapeIndex = -1;
    shapeCrossfade = 1.0f;
}
