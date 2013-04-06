// QueenMaxima, a chess playing program. 
// Copyright (C) 1996-2013 Erik van het Hof and Hermen Reitsma 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// 

#include "defines.h" 
#include "bbtables.h" 

_int64 kingmoves[64] = {
_LL(0x0000000000000302),_LL(0x0000000000000705),_LL(0x0000000000000E0A),_LL(0x0000000000001C14),
_LL(0x0000000000003828),_LL(0x0000000000007050),_LL(0x000000000000E0A0),_LL(0x000000000000C040),
_LL(0x0000000000030203),_LL(0x0000000000070507),_LL(0x00000000000E0A0E),_LL(0x00000000001C141C),
_LL(0x0000000000382838),_LL(0x0000000000705070),_LL(0x0000000000E0A0E0),_LL(0x0000000000C040C0),
_LL(0x0000000003020300),_LL(0x0000000007050700),_LL(0x000000000E0A0E00),_LL(0x000000001C141C00),
_LL(0x0000000038283800),_LL(0x0000000070507000),_LL(0x00000000E0A0E000),_LL(0x00000000C040C000),
_LL(0x0000000302030000),_LL(0x0000000705070000),_LL(0x0000000E0A0E0000),_LL(0x0000001C141C0000),
_LL(0x0000003828380000),_LL(0x0000007050700000),_LL(0x000000E0A0E00000),_LL(0x000000C040C00000),
_LL(0x0000030203000000),_LL(0x0000070507000000),_LL(0x00000E0A0E000000),_LL(0x00001C141C000000),
_LL(0x0000382838000000),_LL(0x0000705070000000),_LL(0x0000E0A0E0000000),_LL(0x0000C040C0000000),
_LL(0x0003020300000000),_LL(0x0007050700000000),_LL(0x000E0A0E00000000),_LL(0x001C141C00000000),
_LL(0x0038283800000000),_LL(0x0070507000000000),_LL(0x00E0A0E000000000),_LL(0x00C040C000000000),
_LL(0x0302030000000000),_LL(0x0705070000000000),_LL(0x0E0A0E0000000000),_LL(0x1C141C0000000000),
_LL(0x3828380000000000),_LL(0x7050700000000000),_LL(0xE0A0E00000000000),_LL(0xC040C00000000000),
_LL(0x0203000000000000),_LL(0x0507000000000000),_LL(0x0A0E000000000000),_LL(0x141C000000000000),
_LL(0x2838000000000000),_LL(0x5070000000000000),_LL(0xA0E0000000000000),_LL(0x40C0000000000000)
};

_int64 queenmoves[64] = {
_LL(0x81412111090503FE),_LL(0x02824222120A07FD),_LL(0x0404844424150EFB),_LL(0x08080888492A1CF7),
_LL(0x10101011925438EF),_LL(0x2020212224A870DF),_LL(0x404142444850E0BF),_LL(0x8182848890A0C07F),
_LL(0x412111090503FE03),_LL(0x824222120A07FD07),_LL(0x04844424150EFB0E),_LL(0x080888492A1CF71C),
_LL(0x101011925438EF38),_LL(0x20212224A870DF70),_LL(0x4142444850E0BFE0),_LL(0x82848890A0C07FC0),
_LL(0x2111090503FE0305),_LL(0x4222120A07FD070A),_LL(0x844424150EFB0E15),_LL(0x0888492A1CF71C2A),
_LL(0x1011925438EF3854),_LL(0x212224A870DF70A8),_LL(0x42444850E0BFE050),_LL(0x848890A0C07FC0A0),
_LL(0x11090503FE030509),_LL(0x22120A07FD070A12),_LL(0x4424150EFB0E1524),_LL(0x88492A1CF71C2A49),
_LL(0x11925438EF385492),_LL(0x2224A870DF70A824),_LL(0x444850E0BFE05048),_LL(0x8890A0C07FC0A090),
_LL(0x090503FE03050911),_LL(0x120A07FD070A1222),_LL(0x24150EFB0E152444),_LL(0x492A1CF71C2A4988),
_LL(0x925438EF38549211),_LL(0x24A870DF70A82422),_LL(0x4850E0BFE0504844),_LL(0x90A0C07FC0A09088),
_LL(0x0503FE0305091121),_LL(0x0A07FD070A122242),_LL(0x150EFB0E15244484),_LL(0x2A1CF71C2A498808),
_LL(0x5438EF3854921110),_LL(0xA870DF70A8242221),_LL(0x50E0BFE050484442),_LL(0xA0C07FC0A0908884),
_LL(0x03FE030509112141),_LL(0x07FD070A12224282),_LL(0x0EFB0E1524448404),_LL(0x1CF71C2A49880808),
_LL(0x38EF385492111010),_LL(0x70DF70A824222120),_LL(0xE0BFE05048444241),_LL(0xC07FC0A090888482),
_LL(0xFE03050911214181),_LL(0xFD070A1222428202),_LL(0xFB0E152444840404),_LL(0xF71C2A4988080808),
_LL(0xEF38549211101010),_LL(0xDF70A82422212020),_LL(0xBFE0504844424140),_LL(0x7FC0A09088848281)
};

_int64 rookmoves[64] = {
_LL(0x01010101010101FE),_LL(0x02020202020202FD),_LL(0x04040404040404FB),_LL(0x08080808080808F7),
_LL(0x10101010101010EF),_LL(0x20202020202020DF),_LL(0x40404040404040BF),_LL(0x808080808080807F),
_LL(0x010101010101FE01),_LL(0x020202020202FD02),_LL(0x040404040404FB04),_LL(0x080808080808F708),
_LL(0x101010101010EF10),_LL(0x202020202020DF20),_LL(0x404040404040BF40),_LL(0x8080808080807F80),
_LL(0x0101010101FE0101),_LL(0x0202020202FD0202),_LL(0x0404040404FB0404),_LL(0x0808080808F70808),
_LL(0x1010101010EF1010),_LL(0x2020202020DF2020),_LL(0x4040404040BF4040),_LL(0x80808080807F8080),
_LL(0x01010101FE010101),_LL(0x02020202FD020202),_LL(0x04040404FB040404),_LL(0x08080808F7080808),
_LL(0x10101010EF101010),_LL(0x20202020DF202020),_LL(0x40404040BF404040),_LL(0x808080807F808080),
_LL(0x010101FE01010101),_LL(0x020202FD02020202),_LL(0x040404FB04040404),_LL(0x080808F708080808),
_LL(0x101010EF10101010),_LL(0x202020DF20202020),_LL(0x404040BF40404040),_LL(0x8080807F80808080),
_LL(0x0101FE0101010101),_LL(0x0202FD0202020202),_LL(0x0404FB0404040404),_LL(0x0808F70808080808),
_LL(0x1010EF1010101010),_LL(0x2020DF2020202020),_LL(0x4040BF4040404040),_LL(0x80807F8080808080),
_LL(0x01FE010101010101),_LL(0x02FD020202020202),_LL(0x04FB040404040404),_LL(0x08F7080808080808),
_LL(0x10EF101010101010),_LL(0x20DF202020202020),_LL(0x40BF404040404040),_LL(0x807F808080808080),
_LL(0xFE01010101010101),_LL(0xFD02020202020202),_LL(0xFB04040404040404),_LL(0xF708080808080808),
_LL(0xEF10101010101010),_LL(0xDF20202020202020),_LL(0xBF40404040404040),_LL(0x7F80808080808080)
}; 

_int64 bishopmoves[64] = {
_LL(0x8040201008040200),_LL(0x0080402010080500),_LL(0x0000804020110A00),_LL(0x0000008041221400),
_LL(0x0000000182442800),_LL(0x0000010204885000),_LL(0x000102040810A000),_LL(0x0102040810204000),
_LL(0x4020100804020002),_LL(0x8040201008050005),_LL(0x00804020110A000A),_LL(0x0000804122140014),
_LL(0x0000018244280028),_LL(0x0001020488500050),_LL(0x0102040810A000A0),_LL(0x0204081020400040),
_LL(0x2010080402000204),_LL(0x4020100805000508),_LL(0x804020110A000A11),_LL(0x0080412214001422),
_LL(0x0001824428002844),_LL(0x0102048850005088),_LL(0x02040810A000A010),_LL(0x0408102040004020),
_LL(0x1008040200020408),_LL(0x2010080500050810),_LL(0x4020110A000A1120),_LL(0x8041221400142241),
_LL(0x0182442800284482),_LL(0x0204885000508804),_LL(0x040810A000A01008),_LL(0x0810204000402010),
_LL(0x0804020002040810),_LL(0x1008050005081020),_LL(0x20110A000A112040),_LL(0x4122140014224180),
_LL(0x8244280028448201),_LL(0x0488500050880402),_LL(0x0810A000A0100804),_LL(0x1020400040201008),
_LL(0x0402000204081020),_LL(0x0805000508102040),_LL(0x110A000A11204080),_LL(0x2214001422418000),
_LL(0x4428002844820100),_LL(0x8850005088040201),_LL(0x10A000A010080402),_LL(0x2040004020100804),
_LL(0x0200020408102040),_LL(0x0500050810204080),_LL(0x0A000A1120408000),_LL(0x1400142241800000),
_LL(0x2800284482010000),_LL(0x5000508804020100),_LL(0xA000A01008040201),_LL(0x4000402010080402),
_LL(0x0002040810204080),_LL(0x0005081020408000),_LL(0x000A112040800000),_LL(0x0014224180000000),
_LL(0x0028448201000000),_LL(0x0050880402010000),_LL(0x00A0100804020100),_LL(0x0040201008040201)
};

_int64 knightmoves[64] = {
_LL(0x0000000000020400),_LL(0x0000000000050800),_LL(0x00000000000A1100),_LL(0x0000000000142200),
_LL(0x0000000000284400),_LL(0x0000000000508800),_LL(0x0000000000A01000),_LL(0x0000000000402000),
_LL(0x0000000002040004),_LL(0x0000000005080008),_LL(0x000000000A110011),_LL(0x0000000014220022),
_LL(0x0000000028440044),_LL(0x0000000050880088),_LL(0x00000000A0100010),_LL(0x0000000040200020),
_LL(0x0000000204000402),_LL(0x0000000508000805),_LL(0x0000000A1100110A),_LL(0x0000001422002214),
_LL(0x0000002844004428),_LL(0x0000005088008850),_LL(0x000000A0100010A0),_LL(0x0000004020002040),
_LL(0x0000020400040200),_LL(0x0000050800080500),_LL(0x00000A1100110A00),_LL(0x0000142200221400),
_LL(0x0000284400442800),_LL(0x0000508800885000),_LL(0x0000A0100010A000),_LL(0x0000402000204000),
_LL(0x0002040004020000),_LL(0x0005080008050000),_LL(0x000A1100110A0000),_LL(0x0014220022140000),
_LL(0x0028440044280000),_LL(0x0050880088500000),_LL(0x00A0100010A00000),_LL(0x0040200020400000),
_LL(0x0204000402000000),_LL(0x0508000805000000),_LL(0x0A1100110A000000),_LL(0x1422002214000000),
_LL(0x2844004428000000),_LL(0x5088008850000000),_LL(0xA0100010A0000000),_LL(0x4020002040000000),
_LL(0x0400040200000000),_LL(0x0800080500000000),_LL(0x1100110A00000000),_LL(0x2200221400000000),
_LL(0x4400442800000000),_LL(0x8800885000000000),_LL(0x100010A000000000),_LL(0x2000204000000000),
_LL(0x0004020000000000),_LL(0x0008050000000000),_LL(0x00110A0000000000),_LL(0x0022140000000000),
_LL(0x0044280000000000),_LL(0x0088500000000000),_LL(0x0010A00000000000),_LL(0x0020400000000000)
};