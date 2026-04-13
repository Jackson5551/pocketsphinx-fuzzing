# PocketSphinx Attack Surface Map

## Input Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                      PocketSphinx Library                        │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                    Main Decoder                            │ │
│  │                  (ps_decoder_t)                            │ │
│  └───────┬───────────────┬─────────────┬──────────────┬───────┘ │
│          │               │             │              │          │
│          ▼               ▼             ▼              ▼          │
│  ┌──────────────┐ ┌────────────┐ ┌──────────┐ ┌───────────┐    │
│  │   Acoustic   │ │  Language  │ │ Grammar  │ │   Audio   │    │
│  │    Model     │ │   Model    │ │  Parser  │ │  Parser   │    │
│  │   Loader     │ │   Loader   │ │          │ │           │    │
│  └──────┬───────┘ └─────┬──────┘ └────┬─────┘ └─────┬─────┘    │
│         │               │              │             │          │
└─────────┼───────────────┼──────────────┼─────────────┼──────────┘
          │               │              │             │
          │               │              │             │
   ┌──────▼──────┐  ┌─────▼──────┐  ┌───▼─────┐  ┌───▼─────┐
   │ *.bin model │  │ *.arpa     │  │ *.jsgf  │  │ *.wav   │
   │ *.mdef      │  │ *.dmp      │  │ *.fsg   │  │ *.nist  │
   │ *.feat      │  │ *.bin      │  │         │  │         │
   └─────────────┘  └────────────┘  └─────────┘  └─────────┘
   Acoustic Models  Language Models  Grammars    Audio Files
   (Priority: LOW)  (Priority: HIGH) (Priority:  (Priority:
                                      CRITICAL)   HIGH)
```

---

## Attack Surface Breakdown

### 🔴 CRITICAL PRIORITY

#### 1. JSGF Grammar Parser
```
┌─────────────────────────────────────────────────────────────┐
│ JSGF Grammar Parser                                         │
│ Files: src/lm/jsgf.c, jsgf_parser.c, jsgf_scanner.c       │
├─────────────────────────────────────────────────────────────┤
│ Input: .jsgf text files                                     │
│                                                             │
│ Data Flow:                                                  │
│   User File                                                 │
│      ↓                                                      │
│   jsgf_parse_file()                                        │
│      ↓                                                      │
│   Flex Scanner (jsgf_scanner.c)                           │
│      ↓                                                      │
│   Bison Parser (jsgf_parser.c)                            │
│      ↓                                                      │
│   Grammar Tree Construction                                 │
│      ↓                                                      │
│   jsgf_t structure                                         │
│                                                             │
│ Vulnerability Hotspots:                                     │
│ • Recursive rule parsing → Stack exhaustion                │
│ • String token handling → Buffer overflows                 │
│ • Weight parsing → Float edge cases                        │
│ • Import resolution → Path traversal?                      │
│ • Tree construction → Use-after-free                       │
│                                                             │
│ Code Patterns Observed:                                     │
│ • ckd_salloc() for string allocation                       │
│ • Recursive descent through grammar rules                   │
│ • Hash table for rule storage                              │
│ • Manual memory management in error paths                   │
└─────────────────────────────────────────────────────────────┘
```

**Why Critical:**
- Complex parser with Flex/Bison (historically buggy)
- Recursive structure prone to stack issues
- Direct user input, often untrusted
- Error handling paths may leak memory/corrupt state

**Attack Vectors:**
- Deeply nested rules: `<a> = <b>; <b> = <c>; ... <z> = <a>;`
- Large alternative lists: `<x> = (w1 | w2 | ... | w999999);`
- Malformed weights: `<x> = /AAAA/ word;`
- Special characters in rule names: `<$%^&*> = test;`

---

### 🔴 CRITICAL PRIORITY

#### 2. N-gram Language Model Parser
```
┌─────────────────────────────────────────────────────────────┐
│ N-gram Model Parser                                          │
│ Files: src/lm/ngram_model.c, ngram_model_trie.c            │
├─────────────────────────────────────────────────────────────┤
│ Input: .arpa (text), .dmp/.bin (binary), .gz/.bz2          │
│                                                             │
│ Data Flow (ARPA):                                           │
│   User File                                                 │
│      ↓                                                      │
│   ngram_model_read()                                       │
│      ↓                                                      │
│   Format Detection (magic bytes)                           │
│      ↓                                                      │
│   ngram_model_trie_read_arpa()                            │
│      ↓                                                      │
│   Parse \data\ section                                     │
│      ↓                                                      │
│   Allocate trie structures                                 │
│      ↓                                                      │
│   Parse \N-grams\ sections                                 │
│      ↓                                                      │
│   Quantize/compress                                        │
│                                                             │
│ Data Flow (Binary):                                         │
│   User File                                                 │
│      ↓                                                      │
│   ngram_model_trie_read_bin()                             │
│      ↓                                                      │
│   Read header (counts, sizes)                              │
│      ↓                                                      │
│   fread() trie data                                        │
│      ↓                                                      │
│   Byteswap if needed                                       │
│                                                             │
│ Vulnerability Hotspots:                                     │
│ • Size calculations: count * size → Integer overflow       │
│ • fread() of attacker-controlled sizes                     │
│ • Probability parsing: log probabilities                   │
│ • Format confusion: ARPA vs BIN detection                  │
│ • Compression handling: .gz/.bz2 decompression bombs       │
│ • Trie construction: Pointer arithmetic                    │
│                                                             │
│ Code Patterns Observed:                                     │
│ • Direct fread() into structures                           │
│ • calloc(count, size) without overflow check               │
│ • Floating point probability calculations                   │
│ • Endianness handling (byteswap)                           │
└─────────────────────────────────────────────────────────────┘
```

**Why Critical:**
- Multiple format parsers (ARPA text, BIN binary)
- Binary parsing with size fields from attacker
- Integer overflow in size calculations
- Complex trie data structure

**Attack Vectors:**
- ARPA with huge ngram counts: `ngram 1=999999999`
- Binary with malicious size headers
- Negative/zero probability values
- Malformed ARPA sections
- Mixed format confusion attacks

---

### 🟠 HIGH PRIORITY

#### 3. Audio File Parser
```
┌─────────────────────────────────────────────────────────────┐
│ Audio File Parser                                            │
│ Files: src/util/soundfiles.c                                │
├─────────────────────────────────────────────────────────────┤
│ Input: .wav (WAV), .nist (NIST SPHERE)                     │
│                                                             │
│ Data Flow (WAV):                                            │
│   User File                                                 │
│      ↓                                                      │
│   ps_config_soundfile()                                    │
│      ↓                                                      │
│   Read magic: "RIFF" or "NIST"                            │
│      ↓                                                      │
│   ps_config_wavfile()                                      │
│      ↓                                                      │
│   Parse RIFF chunks                                        │
│      ↓                                                      │
│   Read "fmt " chunk                                        │
│      ↓                                                      │
│   Extract: sample rate, channels, bit depth                │
│      ↓                                                      │
│   Seek to "data" chunk                                     │
│                                                             │
│ Data Flow (NIST):                                           │
│   User File                                                 │
│      ↓                                                      │
│   ps_config_nistfile()                                     │
│      ↓                                                      │
│   Parse NIST header (1024 bytes)                           │
│      ↓                                                      │
│   Extract key-value pairs                                  │
│                                                             │
│ Vulnerability Hotspots:                                     │
│ • Chunk size validation → Integer overflows                │
│ • Sample count * bytes → Allocation size calculation       │
│ • Chunk parsing → Missing bounds checks                    │
│ • Format field validation → Out-of-range values            │
│ • Nested chunk handling → Recursive parsing                │
│                                                             │
│ Code Patterns Observed:                                     │
│ • TRY_FREAD macro (error handling)                         │
│ • Direct fread() of header structures                      │
│ • memcmp() for magic byte detection                        │
│ • fseek() based on chunk sizes                             │
└─────────────────────────────────────────────────────────────┘
```

**Why High Priority:**
- WAV/RIFF parsing has long history of bugs
- Header size fields control memory operations
- User-provided audio files are common attack vector
- Less complex than parsers above, but still risky

**Attack Vectors:**
- Huge chunk sizes: `data_size = 0xFFFFFFFF`
- Invalid sample rates: `sample_rate = 0` or `sample_rate = -1`
- Malformed chunk IDs causing parsing confusion
- Nested/overlapping chunks

---

### 🟡 MEDIUM PRIORITY

#### 4. FSG Parser
```
┌─────────────────────────────────────────────────────────────┐
│ FSG (Finite State Grammar) Parser                           │
│ Files: src/lm/fsg_model.c                                   │
├─────────────────────────────────────────────────────────────┤
│ Input: .fsg text files                                      │
│                                                             │
│ Data Flow:                                                  │
│   User File                                                 │
│      ↓                                                      │
│   fsg_model_readfile()                                     │
│      ↓                                                      │
│   Parse FSG_BEGIN header                                   │
│      ↓                                                      │
│   Parse NUM_STATES                                         │
│      ↓                                                      │
│   Allocate state array                                     │
│      ↓                                                      │
│   Parse TRANSITION lines                                   │
│      ↓                                                      │
│   Build state machine graph                                │
│                                                             │
│ Vulnerability Hotspots:                                     │
│ • State count → Array allocation size                      │
│ • State references → Out-of-bounds array access            │
│ • Transition loops → Infinite cycles                       │
│ • Probability parsing → Float edge cases                   │
└─────────────────────────────────────────────────────────────┘
```

**Attack Vectors:**
- Invalid state references: `TRANSITION 999 999 1.0 word`
- Self-loops without proper cycle detection
- Huge state counts: `NUM_STATES 999999999`

---

### 🟡 MEDIUM PRIORITY

#### 5. Dictionary Parser
```
┌─────────────────────────────────────────────────────────────┐
│ Dictionary Parser                                            │
│ Files: src/dict.c                                           │
├─────────────────────────────────────────────────────────────┤
│ Input: Text dictionary files (word phoneme mappings)        │
│                                                             │
│ Format: WORD  P H O N E M E S                              │
│                                                             │
│ Vulnerability Hotspots:                                     │
│ • Line parsing → Buffer overflows                          │
│ • Hash table construction → Collision handling             │
│ • Phoneme string parsing → Format confusion                │
└─────────────────────────────────────────────────────────────┘
```

**Attack Vectors:**
- Very long lines
- Hash collision attacks
- Special characters in words

---

## Comparative Risk Assessment

```
                    Complexity    Exposure    Bug History    Overall Risk
┌──────────────────────────────────────────────────────────────────────────┐
│ JSGF Parser     │ ████████    │ ███████   │ ████████    │ ⚠️  CRITICAL  │
│ N-gram Parser   │ █████████   │ ████████  │ ████████    │ ⚠️  CRITICAL  │
│ Audio Parser    │ ████████    │ ████████  │ █████████   │ ⚠️  HIGH      │
│ FSG Parser      │ █████       │ ████      │ █████       │ 🟡 MEDIUM     │
│ Dict Parser     │ ███         │ ████      │ ████        │ 🟡 MEDIUM     │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Fuzzing Priority Matrix

```
High Impact ↑
            │
            │  ┌──────────┐
            │  │  JSGF    │  ← START HERE
            │  │  N-gram  │
            │  └──────────┘
            │       ┌─────────┐
            │       │  Audio  │
            │       └─────────┘
            │            ┌────┐
            │            │FSG │
            │            └────┘
            │                 ┌────┐
            │                 │Dict│
            │                 └────┘
            └────────────────────────────→
                    Low Complexity    High Complexity
```

**Recommendation:**
1. Start with **JSGF** and **N-gram** (highest risk)
2. Add **Audio** after 24 hours
3. Include **FSG** and **Dict** for completeness

---

## Code-Level Hotspots

### Functions to Monitor

```c
// JSGF Parser
jsgf_parse_file()           // Entry point
yyparse()                   // Bison parser (recursion!)
jsgf_atom_new()            // String allocation
jsgf_rule_free()           // Memory cleanup (UAF risk)

// N-gram Parser
ngram_model_read()          // Format detection
ngram_model_trie_read_arpa() // Text parsing
ngram_model_trie_read_bin()  // Binary parsing (dangerous!)
lm_trie_quant_read()        // Quantized trie reading

// Audio Parser
ps_config_wavfile()         // WAV parsing
ps_config_nistfile()        // NIST parsing
TRY_FREAD()                // Macro - check error handling

// FSG Parser
fsg_model_readfile()        // Entry point
fsg_model_add_trans()       // State validation needed

// Dictionary Parser
dict_read()                 // Entry point
hash_table_enter()          // Collision handling
```

---

## Memory Allocation Patterns

```c
// Common allocation pattern (vulnerable to overflow):
ngram_count = read_int32(file);  // Attacker controlled
ngrams = calloc(ngram_count, sizeof(ngram_t));  // Overflow?

// Better:
if (ngram_count > MAX_REASONABLE_COUNT) return ERROR;
if (ngram_count > SIZE_MAX / sizeof(ngram_t)) return ERROR;
ngrams = calloc(ngram_count, sizeof(ngram_t));
```

---

## Summary

**Top 3 Attack Surfaces** (in order):

1. **JSGF Grammar Parser**
   - Most complex
   - Recursive parsing
   - Bison/Flex implementation
   - Directly exposed to user input

2. **N-gram Model Parser**
   - Multiple formats (text + binary)
   - Size calculations from untrusted input
   - Complex trie structures
   - Compression handling

3. **Audio File Parser**
   - Historical vulnerability history in RIFF/WAV
   - Header-based memory operations
   - Common attack vector (user audio files)

Start fuzzing these three for maximum bug discovery potential.
