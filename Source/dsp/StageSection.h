#pragma once

// StageSection interface
// Each DSP stage implements prepare/process/reset by convention.
// No virtual base class -- concrete classes to avoid virtual dispatch overhead.
// See Pattern 2 in 01-RESEARCH.md.
