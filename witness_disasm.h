struct AutoArray<int> {
  int32_t size;
  int32_t[] data; // offset 8
};

struct EntityMachinePanel {
  int type; // Offset 0x8
  int panelId; // Offset 0x10
  byte something; // Offset 0x14
  int something; // Offset 0x20
  float x; // Offset 0x24
  float y; // Offset 0x28
  float z; // Offset 0x2C
  float yaw?; // Offset 0x30
  float patch?; // Offset 0x34
  float roll?; // Offset 0x38
  
  char* name; // Offset 0x220
  int32_t numSegments; // Offset 0x230 // 220
  int32_t solvedState; // Offset 0x288
  float isActive; // Offset 0x2A8 // 2A0
  int32_t numDots; // Offset 0x3B8
  int32_t numEdgesPlusOne; // offset 0x3BC
  int32_t[] dots; // Offset 0x3D0
  int32_t[] cells; // offset 0x420
  int32_t numCells; // offset 0x438
  // 0: Asymmetric
  int64_t symmetryFlags; // Offset 0x440
  int32_t panelWidthPlusOne; // offset 0x448
  int32_t panelHeightPlusOne; // offset 0x44C
};

// Global Random_Generator
void shuffle<T>(struct AutoArray<T>* arr) {
  if (arr0->size <= 0) return;
  
  for (int i=0; i<arr->size; i++) {
    int64_t randIndex = Random_Generator::get() % arr->size;
    int64_t randIndex2 = Random_Generator::get() % arr->size;

    int32_t tmp = arr->data[randIndex];
    arr->data[randIndex] = arr->data[randIndex2];
    arr->data[randIndex2] = tmp;
  }
}

int64_t get_empty_decoration_slot(EntityMachinePanel* panel) {
  uint32_t randIndex;
  do {
    randIndex = Random_Generator.get() % panel->numCells;
  } while (panel->cells[randIndex] != 0);
  return randIndex;
}

/*
 0123
 4567
 89AB
 CDEF

 B/4 - E/4 (12/4 - 15/4) = 3-4 = -1
 B%4 - E%4 (12%4 - 15%4) = 4-3 = 1
*/
int64_t get_manhattan_distance(EntityMachinePanel panel, int32_t i1, int32_t i2) {
  int32_t panelWidth = panel->panelWidthPlusOne - 1;
  rowDiff = (i2 - i1) / panelWidth;
  colDiff = (i2 - i1) % panelWidth;
  if (colDiff < 0) {
    colDiff = -colDiff;
  }
  if (rowDiff < 0) {
    rowDiff = -rowDiff;
  }
  return rowDiff + colDiff;
}

/* Auto_Array<int>::add */
void Auto_Array<int>::add(int64_t item);

void cut_edge(EntityMachinePanel* panel, int64_t texture_mask_0, int64_t texture_mask_1);
void cut_edge_asymmetric(EntityMachinePanel* panel, int64_t texture_mask_0, int64_t texture_mask_1);
// Might want to re-do this one
int64_t cut_random_edges(EntityMachinePanel* panel, uint32_t orig_num_dots, uint32_t num_cuts, int64_t /*Auto_Array<int>*/ results) {
  uint32_t v7 = orig_num_dots;

  int32_t numEdgesPlusOne = panel->numEdgesPlusOne;
  if (numEdgesPlusOne == 0) return 0;
  
  for (uint32_t i = 0; i < num_cuts; i++) {
    int64_t randomEdge = Random_Generator.get() % numEdgesPlusOne;
    uint32_t x = panel->f984[randomEdge];
    uint32_t y = panel->f992[randomEdge];
    if (x < v7 && y < v7) { // dumb sanity check?
      if (results) {
        results.add(x);
        results.add(y);
      }
      if (panel->symmetry != 0) {
        panel.cut_edge(randomEdge, x, y);
      } else {
        panel.cut_edge_asymmetric(randomEdge, x, y);
      }
    }
  }
  return num_cuts;
}

int64_t make_a_shaper() {
  int64_t shaperSize = Random_Generator.get() % 3 + 3;
  shaper_begin();
  for (; shaperSize > 0; shaperSize--) {
    if (Random_Generator.get() & 1) {
      shaper_go(0, 1);
    } else {
      shaper_go(1, 0);
    }
  }
  uint32_t shaper_result = shaper_end();
  
  uint32_t numRotations = Random_Generator.get() & 3; // 0 to 3
  for (; numRotations>0; numRotations--) {
    shaper_result = rotate_shape(shaper_result);
  }
  
  // Some encoding of shapers, not really sure what
  if (shaper_result == 0) return shaper_result
  // Removing trailing 0s (base 16)
  while (shaper_result & 0xF == 0 && shaper_result != 0)
    shaper_result = shaper_result >> 4;
  }
  if (shaper_result == 0) return shaper_result
  while (shaper_result & 0x1111 != 0 && shaper_result != 0) {
    shaper_result = (shaper_result >> 1) & 0x7777;
  }
  return shaper_result;
}

// 0: Do nothing 1: Puzzle impossible, don't continue
// This checks for invalid L shapes with 3 colors. It looks like there are cell objects for each color of stone, so they can be directly compared.
int64_t test_stones_early_fail(EntityMachinePanel panel) {
  width = panel->panelWidthPlusOne - 1;
  height = (uint32_t)(panel->panelHeightPlusOne - 1);
  if (panel->f1104 & 0x4000) {
    width++;
  }
  if (width <= 1) return 0;
  heightMinusOne = height - 1;
  if (heightMinusOne <= 0) return 0;
  for (int32_t x = 0; x < width - 1; x++) {
    int32_t y = 0;
    panelCells = panel->cells;
    x_copy1 = x;
    while (1) {
      cell1_copy = panelCells[x];
      if ((cell1_copy & 0xf00) == 0x100) {
        cell1 = cell1_copy;
      } else {
        cell1 = 0;
      }
      cell2_copy = (panelCells + x)[1];
      if ((cell2_copy & 0xf00) == 0x100) {
        cell2 = cell2_copy;
      } else {
        cell2 = 0;
      }
      cell3_copy = panelCells[x + width];
      if ((cell3_copy & 0xf00) == 0x100) {
        cell3 = cell3_copy;
      } else {
        cell3 = 0;
      }
      cell4_copy = (panelCells + x + width)[1];
      if ((cell4_copy & 0xf00) == 0x100) {
        cell4 = cell4_copy;
      } else {
        cell4 = 0;
      }
      if (cell1 && !cell2) {
        addr_0x1400c50d0_22:
        if (cell3 && !cell4) {
          addr_0x1400c511f_24:
          ++y;
          x_copy1 += width;
          if (y < heightMinusOne) 
            continue;
          else
            break;
        }
        if (cell3 && cell4) {
          if (cell3 != cell4) {
            if (cell1 && cell1 != cell3 && cell1 != cell4) {
              return 1;
            }
            if (cell2 && cell2 != cell3 && cell2 != cell4) {
              return 1;
            }
          }
          if (cell1 && cell4 != cell1) {
            if (cell2 && cell2 != cell4 && cell2 != cell1) {
              return 1;
            }
            if (cell3 && cell3 != cell4 && cell3 != cell1) {
              return 1;
            }
          }
          goto addr_0x1400c511f_24;
        }
      }
      if (cell1 && cell2 && cell1 != cell2) { 
        if (cell3 && cell3 != cell1 && cell3 != cell2) {
          return 1;
        }
        if (cell4 && cell4 != cell1 && cell4 != cell2) 
          return 1;
        }
      }
      if (cell2 && !cell3) {
        if (cell4 && cell1 && cell4 != cell1) {
          if (cell2 && cell2 != cell4 && cell2 != cell1) {
            return 1;
          }
          if (cell3 && cell3 != cell4 && cell3 != cell1) {
            return 1;
          }
        }
        goto addr_0x1400c511f_24;
      }
      if (cell2 && cell3) {
        if (cell2 != cell3) {
          if (cell4 && cell4 != cell2 && cell4 != cell3) {
            return 1;
          }
          if (cell1 && cell1 != cell2 && cell1 != cell3) {
            return 1;
          }
        }
        goto addr_0x1400c50d0_22;
      }
    }
  }
  return 0;
}

// Right door
void fun_140154fb0() {
    do {
        panel.clear_dots();
        panel.make_n_by_n(5, true);
        panel.init_decorations();

        panel->dots->f16 = panel->dots->f16 | 2;

        int32_t emptyDot1 = panel.get_empty_dot_spot();
        panel->dots[emptyDot1] = panel->dots[emptyDot1] | 32;
        int32_t emptyDot2 = panel.get_empty_dot_spot();
        panel->dots[emptyDot2] = panel->dots[emptyDot2] | 32;
        
        panel.cut_random_edges(panel->numDots, 8);
        panel.add_finisher(20, 0xBF800000, true);

        int32_t emptySlot1 = panel.get_empty_decoration_slot();
        panel->cells[emptySlot1] = stone_color1;
        int32_t emptySlot2 = panel.get_empty_decoration_slot();
        panel->cells[emptySlot2] = stone_color1;
        int32_t emptySlot3 = panel.get_empty_decoration_slot();
        panel->cells[emptySlot3] = stone_color2;
        int32_t emptySlot4 = panel.get_empty_decoration_slot();
        panel->cells[emptySlot4] = stone_color2;

        panel->f1104 = panel->f1104 | 0x108;
    } while (!panel.is_solvable());
}


struct EntityMachinePanel {
    int64_t f256; // Offset 256
    int64_t f264; // Offset 264
    int32_t f272; // Offset 272
};

void gamelib_free_string(int64_t panel, void* rdx, ...);

int64_t g1405b28d0 = 0x22de4b80;

/* Random_Generator::get */
uint32_t Random_Generator_get(int64_t panel, void* rdx);

int64_t mprintf(int64_t panel, void* rdx, ...);

void _auto_array_expand(void* panel, void* rdx, int64_t r8, int64_t r9);

void shuffle_integers(int64_t panel, int64_t rdx, int64_t r8, int64_t r9);

struct s1 {
    int8_t[8] pad8;
    int64_t f8;
    int32_t f16;
    int8_t[4] pad24;
    int64_t f24;
    int8_t[180] pad212;
    int32_t f212;
    int8_t[328] pad544;
    int64_t name;
    int8_t[148] pad700;
    int32_t f700;
};

struct s1* find_labeled_entity(int64_t panel, void* rdx, int64_t r8, int64_t r9);

void _auto_array_deallocate(void* panel, void* rdx, int64_t r8, int64_t r9);

void sprintf(void* panel, int64_t rdx, int64_t r8, int64_t r9);

struct s2 {
    int8_t[8] pad8;
    int32_t f8;
    int8_t[4] pad16;
    int32_t f16;
    int8_t[4] pad24;
    struct s1** f24;
};

struct s2* globals = (struct s2*)0x21888630;

void* clone_entity_with_id(int64_t panel, struct s1* rdx, int64_t r8, int64_t r9);

/* Entity_Manager::schedule_for_destruction */
void Entity_Manager_schedule_for_destruction(int64_t panel, struct s1* rdx, int64_t r8, int64_t r9);

/* Entity_Manager::register_portable */
void Entity_Manager_register_portable(int64_t panel, void* rdx, int64_t r8, int64_t r9);

/* Entity_Record_Player::reroll_lotus_eater_stuff */
void Entity_Record_Player_reroll_lotus_eater_stuff(struct s0* panel, void* rdx) {
  void* rbp3;
  uint32_t eax7;
  int64_t rdx8;
  void* rdx9;
  int64_t rcx12;
  uint32_t eax13;
  int64_t rdx14;
  void* rdx15;
  int64_t rax17;
  int32_t ebx18;
  int64_t v19;
  int64_t v20;
  int64_t r8_21;
  int64_t rax22;
  int64_t r8_23;
  int32_t edi31;
  int64_t r8_32;
  void* rdx36;
  struct s2* rbx38;
  int32_t eax39;

  rbp3 = (void*)((int64_t)__zero_stack_offset() - 40);

  gamelib_free_string(panel->f256);
  rdx9 = Random_Generator.get() % 3;
  panel->f256 = mprintf("lotus_onlyone_%d", rdx9);

  gamelib_free_string(panel->f264);
  rdx15 = Random_Generator.get() % 3;
  panel->f264 = mprintf("lotus_onlyone_tricolor_%d", rdx15);

  rax17 = 0;
  ebx18 = 0;
  v19 = 0;
  v20 = 0;
  while (1) {
    r8_21 = rax17 + 1;
    if (0 < r8_21) {
      if (0 > r8_21) {
        r8_21 = 0;
      }
      _auto_array_expand((int64_t)rbp3 - 72, (int64_t)rbp3 - 76, r8_21, 4);
      rax17 = v19;
    }
    *(int32_t*)(rax17 * 4) = ebx18;
    rax22 = v20;
    v19++;
    r8_23 = rax22 + 1;
    if (0 < r8_23) {
      if (0 > r8_23) {
        r8_23 = 0;
      }
      _auto_array_expand((int64_t)rbp3 - 88, (int64_t)rbp3 - 92, r8_23, 4);
      rax22 = v20;
    }
    *(int32_t*)(rax22 * 4) = ebx18;
    v20++;
    ebx18++;
    if (ebx18 >= 4) 
      break;
    rax17 = v19;
  }
  shuffle_integers(0, v19, r8_23, 4);
  shuffle_integers(0, v20, r8_23, 4);
  startScrambleCable = find_labeled_entity(ptype_Power_Cable, "lotus_start_scramble");
  endScrambleCable = find_labeled_entity(ptype_Power_Cable, "lotus_end_scramble");
  if (!startScrambleCable || !endScrambleCable) return;

  for (int64_t r15_30 = 0; r15_30 < 4; r15_30++) {
    edi31 = *(int32_t*)(r15_30 * 4); // This actually indexes into v19
    r8_32 = *(int32_t*)(r15_30 * 4); // This actually indexes into v20
    randomName = sprintf("lotus_scramble_%d", r8_32);
    randomPanel = find_labeled_entity(ptype_Machine_Panel, randomName);
    if (randomPanel) {
      gamelib_free_string(randomPanel->name);
      randomPanel->name = mprintf("lotus_scramble_%d", edi31);
      if (startScrambleCable) {
        startScrambleCable->f212 = randomPanel->f16;
      }
      eax39 = randomPanel->f700 - globals->type;
      tmp = globals->f24[eax39];
      if (
        !randomPanel->f700 ||
        eax39 < 0 ||
        eax39 >= globals->f16 ||
        tmp == 0 ||
        tmp->type != ptype_Power_Cable)
      ) {
        startScrambleCable = 0;
      }
      if (r15_30 == 3 && startScrambleCable) {
        startScrambleCable->f212 = endScrambleCable->f16;
      }
      entityManager = randomPanel->f24;
      randomPanelClone = entityManager.clone_entity_with_id(randomPanel);
      entityManager.schedule_for_destruction(randomPanel, 2);
      entityManager.register_portable(randomPanelClone, 2);
    }
  }
  panel->f272 = panel->f272 + 1;
  return;
}

// rcx = panel
// rdx = 2
// r8d = 60
// r9d = 36
void add_exactly_this_many_bisection_dots() {
  rax = globals
  rsi = arg1
  

0000000140156619          | mov rax,qword ptr ds:[0x1405B28D0]                                 | panel_puzzles_lotus.cpp:207
0000000140156620          | mov rsi,qword ptr ss:[rsp+0xB8]                                    | panel_puzzles_lotus.cpp:209
0000000140156628          | mov rdi,qword ptr ss:[rsp+0xB0]                                    |
0000000140156630          | lea r14d,qword ptr ds:[r9-0x1]                                     |
0000000140156634          | mov qword ptr ss:[rsp+0x90],rax                                    |
000000014015663C          | mov r15d,edx                                                       |
000000014015663F          | mov r13,rcx                                                        |
0000000140156642          | xor r12d,r12d                                                      |
0000000140156645          | mov dword ptr ss:[rsp+0xA8],r14d                                   |
000000014015664D          | nop dword ptr ds:[rax],eax                                         |
0000000140156650          | mov rcx,rax                                                        | panel_puzzles_lotus.cpp:211
0000000140156653          | call <witness64_d3d11.public: unsigned long __cdecl Random_Generat |
0000000140156658          | xor edx,edx                                                        |
000000014015665A          | div dword ptr ss:[rsp+0xA0]                                        |
0000000140156661          | mov rax,qword ptr ds:[r13+0x3D8]                                   | panel_puzzles_lotus.cpp:212
0000000140156668          | movsxd rcx,edx                                                     |
000000014015666B          | movsxd rbp,dword ptr ds:[rax+rcx*4]                                |
000000014015666F          | mov rax,qword ptr ds:[r13+0x3E0]                                   | panel_puzzles_lotus.cpp:213
0000000140156676          | movsxd rbx,dword ptr ds:[rax+rcx*4]                                |
000000014015667A          | mov rax,qword ptr ss:[rsp+0x90]                                    | panel_puzzles_lotus.cpp:216
0000000140156682          | cmp ebp,r14d                                                       |
0000000140156685          | jge witness64_d3d11.140156650                                      |
0000000140156687          | cmp ebx,r14d                                                       |
000000014015668A          | jge witness64_d3d11.140156650                                      |
000000014015668C          | test ebp,ebp                                                       | panel_puzzles_lotus.cpp:221
000000014015668E          | je witness64_d3d11.140156650                                       |
0000000140156690          | test ebx,ebx                                                       |
0000000140156692          | je witness64_d3d11.140156650                                       |
0000000140156694          | xor eax,eax                                                        | panel_puzzles_lotus.cpp:224
0000000140156696          | xor r9d,r9d                                                        |
0000000140156699          | mov r8d,ebx                                                        |
000000014015669C          | mov qword ptr ss:[rsp+0x28],rax                                    |
00000001401566A1          | mov edx,ebp                                                        |
00000001401566A3          | mov rcx,r13                                                        |
00000001401566A6          | mov qword ptr ss:[rsp+0x20],rax                                    |
00000001401566AB          | call <witness64_d3d11.bool __cdecl bisect_for_touch_me(struct Enti |
00000001401566B0          | test rdi,rdi                                                       | panel_puzzles_lotus.cpp:226
00000001401566B3          | je witness64_d3d11.140156731                                       |
00000001401566B5          | mov r8d,dword ptr ds:[rdi]                                         | panel_puzzles_lotus.cpp:227
00000001401566B8          | mov eax,dword ptr ds:[rdi+0x4]                                     |
00000001401566BB          | inc r8d                                                            |
00000001401566BE          | cmp eax,r8d                                                        |
00000001401566C1          | jge witness64_d3d11.1401566DF                                      |
00000001401566C3          | add eax,eax                                                        |
00000001401566C5          | lea rcx,qword ptr ds:[rdi+0x8]                                     |
00000001401566C9          | lea rdx,qword ptr ds:[rdi+0x4]                                     |
00000001401566CD          | cmp eax,r8d                                                        |
00000001401566D0          | mov r9d,0x4                                                        |
00000001401566D6          | cmovg r8d,eax                                                      |
00000001401566DA          | call <witness64_d3d11.void __cdecl _auto_array_expand(void **, int |
00000001401566DF          | movsxd rcx,dword ptr ds:[rdi]                                      |
00000001401566E2          | mov rax,qword ptr ds:[rdi+0x8]                                     |
00000001401566E6          | mov dword ptr ds:[rax+rcx*4],ebp                                   |
00000001401566E9          | inc dword ptr ds:[rdi]                                             |
00000001401566EB          | mov r8d,dword ptr ds:[rdi]                                         |
00000001401566EE          | mov eax,dword ptr ds:[rdi+0x4]                                     | panel_puzzles_lotus.cpp:228
00000001401566F1          | inc r8d                                                            |
00000001401566F4          | cmp eax,r8d                                                        |
00000001401566F7          | jge witness64_d3d11.140156715                                      |
00000001401566F9          | add eax,eax                                                        |
00000001401566FB          | lea rdx,qword ptr ds:[rdi+0x4]                                     |
00000001401566FF          | lea rcx,qword ptr ds:[rdi+0x8]                                     |
0000000140156703          | cmp eax,r8d                                                        |
0000000140156706          | mov r9d,0x4                                                        |
000000014015670C          | cmovg r8d,eax                                                      |
0000000140156710          | call <witness64_d3d11.void __cdecl _auto_array_expand(void **, int |
0000000140156715          | movsxd rcx,dword ptr ds:[rdi]                                      |
0000000140156718          | mov rax,qword ptr ds:[rdi+0x8]                                     |
000000014015671C          | mov r14d,dword ptr ss:[rsp+0xA8]                                   |
0000000140156724          | mov r15d,dword ptr ss:[rsp+0x98]                                   |
000000014015672C          | mov dword ptr ds:[rax+rcx*4],ebx                                   |
000000014015672F          | inc dword ptr ds:[rdi]                                             |
0000000140156731          | test rsi,rsi                                                       | panel_puzzles_lotus.cpp:231
0000000140156734          | je witness64_d3d11.1401567D1                                       |
000000014015673A          | mov rdx,qword ptr ds:[r13+0x3C8]                                   | panel_puzzles_lotus.cpp:232
0000000140156741          | mov rcx,qword ptr ds:[rdx+rbp*8]                                   |
0000000140156745          | mov qword ptr ss:[rsp+0xB0],rcx                                    |
000000014015674D          | mov rcx,qword ptr ds:[rdx+rbx*8]                                   | panel_puzzles_lotus.cpp:233
0000000140156751          | lea rdx,qword ptr ss:[rsp+0x38]                                    | panel_puzzles_lotus.cpp:234
0000000140156756          | mov qword ptr ss:[rsp+0xB8],rcx                                    |
000000014015675E          | lea rcx,qword ptr ss:[rsp+0x30]                                    |
0000000140156763          | movss xmm0,dword ptr ss:[rsp+0xB8]                                 |
000000014015676C          | movss xmm1,dword ptr ss:[rsp+0xBC]                                 |
0000000140156775          | subss xmm0,dword ptr ss:[rsp+0xB0]                                 |
000000014015677E          | subss xmm1,dword ptr ss:[rsp+0xB4]                                 |
0000000140156787          | movss dword ptr ss:[rsp+0x30],xmm0                                 |
000000014015678D          | movss dword ptr ss:[rsp+0x34],xmm1                                 |
0000000140156793          | call <witness64_d3d11.public: struct Vector2 __cdecl Vector2::unit |
0000000140156798          | mov r8d,dword ptr ds:[rsi]                                         |
000000014015679B          | lea rdx,qword ptr ds:[rsi+0x4]                                     |
000000014015679F          | mov rbx,qword ptr ds:[rax]                                         |
00000001401567A2          | mov eax,dword ptr ds:[rdx]                                         |
00000001401567A4          | inc r8d                                                            |
00000001401567A7          | cmp eax,r8d                                                        |
00000001401567AA          | jge witness64_d3d11.1401567C4                                      |
00000001401567AC          | add eax,eax                                                        |
00000001401567AE          | lea rcx,qword ptr ds:[rsi+0x8]                                     |
00000001401567B2          | mov r9d,0x8                                                        |
00000001401567B8          | cmp eax,r8d                                                        |
00000001401567BB          | cmovg r8d,eax                                                      |
00000001401567BF          | call <witness64_d3d11.void __cdecl _auto_array_expand(void **, int |
00000001401567C4          | movsxd rcx,dword ptr ds:[rsi]                                      |
00000001401567C7          | mov rax,qword ptr ds:[rsi+0x8]                                     |
00000001401567CB          | mov qword ptr ds:[rax+rcx*8],rbx                                   |
00000001401567CF          | inc dword ptr ds:[rsi]                                             |
00000001401567D1          | mov rax,qword ptr ss:[rsp+0x90]                                    | panel_puzzles_lotus.cpp:238
00000001401567D9          | inc r12d                                                           |
00000001401567DC          | cmp r12d,r15d                                                      |
00000001401567DF          | jl witness64_d3d11.140156650                                       |
00000001401567E5          | add rsp,0x48                                                       | panel_puzzles_lotus.cpp:240
00000001401567E9          | pop r15                                                            |
00000001401567EB          | pop r14                                                            |
00000001401567ED          | pop r13                                                            |
00000001401567EF          | pop r12                                                            |
00000001401567F1          | pop rdi                                                            |
00000001401567F2          | pop rsi                                                            |
00000001401567F3          | pop rbp                                                            |
00000001401567F4          | pop rbx                                                            |
00000001401567F5          | ret                                                                |