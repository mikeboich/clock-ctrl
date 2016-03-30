
//`#start header` -- edit after this line, do not edit this line
// ========================================
//
// 
//
// ========================================
`include "cypress.v"
//`#end` -- edit above this line, do not edit this line
// Generated on 08/08/2015 at 07:16
// Component: sin8
module sin8 (
    output [7:0] outp,
	input  [7:0] inp
);

//`#start body` -- edit after this line, do not edit this line

    // parameters // 
    //parameter TableSize = 3'd2; //
    parameter TableSize = 8'd8; //
    
//==============================================================================

    
    // convert input code into sin wave
    generate
    
    if(TableSize==8'd8) // 8bit  
        assign outp = sine3( inp );		   
    else
    if(TableSize==8'd16) // 16bit  
        assign outp = sine4( inp );		   
    else
    if(TableSize==8'd32) // 32bit
        assign outp = sine5( inp );		
    else
    if(TableSize==8'd64) // 64bit 
        assign outp = sine6( inp );		
    else
    if(TableSize==8'd128) // 128bit
        assign outp = sine7( inp );		
    else                // 256bit
        assign outp = sine8( inp );		

    endgenerate
   
    
    //Sin Table calculator:
    //http://www.meraman.com/htmls/en/sinTableOld.html
    //full range=255, zero=127.5, half-wave amplitude 127.5
    
    function [7:0] sine3;	// 4-bit sin table 
    input   [2:0] phase;    // 16 steps
 	begin
	    case( phase )
		    0: sine3=128;  1: sine3=218;  2: sine3=255;  3: sine3=218;  4: sine3=128;  5: sine3=037;  6: sine3=000;  7: sine3=037;
	    default: sine3=255;
	    endcase
    end
    endfunction
    
    
    function [7:0] sine4;	// 4-bit sin table 
    input   [3:0] phase;    // 16 steps
 	begin
	    case( phase )
		    0: sine4=128;  1: sine4=176;  2: sine4=218;  3: sine4=245;  4: sine4=255;  5: sine4=245;  6: sine4=218;  7: sine4=176;
            8: sine4=128;  9: sine4=079; 10: sine4=037; 11: sine4=010; 12: sine4=000; 13: sine4=010; 14: sine4=037; 15: sine4=079;
	    default: sine4=255;
	    endcase
    end
    endfunction
    
    
    function [7:0] sine5;   // 5-bit sin table 
    input   [4:0] phase;    // 32 steps
 	begin
	    case( phase )
		  0: sine5=128;   1: sine5=152;   2: sine5=176;   3: sine5=198;   4: sine5=218;   5: sine5=234;   6: sine5=245;   7: sine5=253;
          8: sine5=255;   9: sine5=253;  10: sine5=245;  11: sine5=234;  12: sine5=218;  13: sine5=198;  14: sine5=176;  15: sine5=152;
         16: sine5=128;  17: sine5=103;  18: sine5=079;  19: sine5=057;  20: sine5=037;  21: sine5=021;  22: sine5=010;  23: sine5=002;
		 24: sine5=000;  25: sine5=002;  26: sine5=010;  27: sine5=021;  28: sine5=037;  29: sine5=057;  30: sine5=079;  31: sine5=103;
	    default: sine5=255;
	 endcase
	end
    endfunction
   
    
    function [7:0] sine6;   // 6-bit sin table
    input   [5:0] phase;    // 64 steps
 	begin
	  case( phase )
		  0: sine6=128;   1: sine6=140;   2: sine6=152;   3: sine6=165;   4: sine6=176;   5: sine6=188;   6: sine6=198;   7: sine6=208;
          8: sine6=218;   9: sine6=226;  10: sine6=234;  11: sine6=240;  12: sine6=245;  13: sine6=250;  14: sine6=253;  15: sine6=254;
         16: sine6=255;  17: sine6=254;  18: sine6=253;  19: sine6=250;  20: sine6=245;  21: sine6=240;  22: sine6=234;  23: sine6=226; 
		 24: sine6=218;  25: sine6=208;  26: sine6=198;  27: sine6=188;  28: sine6=176;  29: sine6=165;  30: sine6=152;  31: sine6=140; 
         32: sine6=128;  33: sine6=115;  34: sine6=103;  35: sine6=090;  36: sine6=079;  37: sine6=067;  38: sine6=057;  39: sine6=047;
         40: sine6=037;  41: sine6=029;  42: sine6=021;  43: sine6=015;  44: sine6=010;  45: sine6=005;  46: sine6=002;  47: sine6=001;
		 48: sine6=000;  49: sine6=001;  50: sine6=002;  51: sine6=005;  52: sine6=010;  53: sine6=015;  54: sine6=021;  55: sine6=029;
         56: sine6=037;  57: sine6=047;  58: sine6=057;  59: sine6=067;  60: sine6=079;  61: sine6=090;  62: sine6=103;  63: sine6=115;
	    default: sine6=255;
	 endcase
	end
    endfunction
   
  
    function [7:0] sine7;   // 7-bit sin table
    input   [6:0] phase;    // 128 steps
 	begin
	  case( phase )
		  0: sine7=128;   1: sine7=134;   2: sine7=140;   3: sine7=146;   4: sine7=152;   5: sine7=158;   6: sine7=165;   7: sine7=170;
          8: sine7=176;   9: sine7=182;  10: sine7=188;  11: sine7=193;  12: sine7=198;  13: sine7=203;  14: sine7=208;  15: sine7=213;
         16: sine7=218;  17: sine7=222;  18: sine7=226;  19: sine7=230;  20: sine7=234;  21: sine7=237;  22: sine7=240;  23: sine7=243;
		 24: sine7=245;  25: sine7=248;  26: sine7=250;  27: sine7=251;  28: sine7=253;  29: sine7=254;  30: sine7=254;  31: sine7=255;
         32: sine7=255;  33: sine7=255;  34: sine7=254;  35: sine7=254;  36: sine7=253;  37: sine7=251;  38: sine7=250;  39: sine7=248;
         40: sine7=245;  41: sine7=243;  42: sine7=240;  43: sine7=237;  44: sine7=234;  45: sine7=230;  46: sine7=226;  47: sine7=222;
		 48: sine7=218;  49: sine7=213;  50: sine7=208;  51: sine7=203;  52: sine7=198;  53: sine7=193;  54: sine7=188;  55: sine7=182;
         56: sine7=176;  57: sine7=170;  58: sine7=165;  59: sine7=158;  60: sine7=152;  61: sine7=146;  62: sine7=140;  63: sine7=134;
         64: sine7=128;  65: sine7=121;  66: sine7=115;  67: sine7=109;  68: sine7=103;  69: sine7=097;  70: sine7=090;  71: sine7=085;
		 72: sine7=079;  73: sine7=073;  74: sine7=067;  75: sine7=062;  76: sine7=057;  77: sine7=052;  78: sine7=047;  79: sine7=042;
         80: sine7=037;  81: sine7=033;  82: sine7=029;  83: sine7=025;  84: sine7=021;  85: sine7=018;  86: sine7=015;  87: sine7=012;
         88: sine7=010;  89: sine7=007;  90: sine7=005;  91: sine7=004;  92: sine7=002;  93: sine7=001;  94: sine7=001;  95: sine7=000; 
		 96: sine7=000;  97: sine7=000;  98: sine7=001;  99: sine7=001; 100: sine7=002; 101: sine7=004; 102: sine7=005; 103: sine7=007;
        104: sine7=010; 105: sine7=012; 106: sine7=015; 107: sine7=018; 108: sine7=021; 109: sine7=025; 110: sine7=029; 111: sine7=033;
        112: sine7=037; 113: sine7=042; 114: sine7=047; 115: sine7=052; 116: sine7=057; 117: sine7=062; 118: sine7=067; 119: sine7=073;
        120: sine7=079; 121: sine7=085; 122: sine7=090; 123: sine7=097; 124: sine7=103; 125: sine7=109; 126: sine7=115; 127: sine7=121; 
	    default: sine7=255;
	 endcase
	end
    endfunction
  
   
    function [7:0] sine8;    // 8-bit sin table
    input   [7:0] phase;    // 256 steps
 	begin
	  case( phase )
		  0: sine8=128;   1: sine8=131;   2: sine8=134;   3: sine8=137;   4: sine8=140;   5: sine8=143;   6: sine8=146;   7: sine8=149;
          8: sine8=152;   9: sine8=155;  10: sine8=158;  11: sine8=162;  12: sine8=165;  13: sine8=167;  14: sine8=170;  15: sine8=173;
         16: sine8=176;  17: sine8=179;  18: sine8=182;  19: sine8=185;  20: sine8=188;  21: sine8=190;  22: sine8=193;  23: sine8=196;
		 24: sine8=198;  25: sine8=201;  26: sine8=203;  27: sine8=206;  28: sine8=208;  29: sine8=211;  30: sine8=213;  31: sine8=215; 
         32: sine8=218;  33: sine8=220;  34: sine8=222;  35: sine8=224;  36: sine8=226;  37: sine8=228;  38: sine8=230;  39: sine8=232;
         40: sine8=234;  41: sine8=235;  42: sine8=237;  43: sine8=238;  44: sine8=240;  45: sine8=241;  46: sine8=243;  47: sine8=244;    
		 48: sine8=245;  49: sine8=246;  50: sine8=248;  51: sine8=249;  52: sine8=250;  53: sine8=250;  54: sine8=251;  55: sine8=252; 
         56: sine8=253;  57: sine8=253;  58: sine8=254;  59: sine8=254;  60: sine8=254;  61: sine8=255;  62: sine8=255;  63: sine8=255;
         64: sine8=255;  65: sine8=255;  66: sine8=255;  67: sine8=255;  68: sine8=254;  69: sine8=254;  70: sine8=254;  71: sine8=253;
		 72: sine8=253;  73: sine8=252;  74: sine8=251;  75: sine8=250;  76: sine8=250;  77: sine8=249;  78: sine8=248;  79: sine8=246;
         80: sine8=245;  81: sine8=244;  82: sine8=243;  83: sine8=241;  84: sine8=240;  85: sine8=238;  86: sine8=237;  87: sine8=235;
         88: sine8=234;  89: sine8=232;  90: sine8=230;  91: sine8=228;  92: sine8=226;  93: sine8=224;  94: sine8=222;  95: sine8=220; 
		 96: sine8=218;  97: sine8=215;  98: sine8=213;  99: sine8=211; 100: sine8=208; 101: sine8=206; 102: sine8=203; 103: sine8=201;
        104: sine8=198; 105: sine8=196; 106: sine8=193; 107: sine8=190; 108: sine8=188; 109: sine8=185; 110: sine8=182; 111: sine8=179;
        112: sine8=176; 113: sine8=173; 114: sine8=170; 115: sine8=167; 116: sine8=165; 117: sine8=162; 118: sine8=158; 119: sine8=155;
        120: sine8=152; 121: sine8=149; 122: sine8=146; 123: sine8=143; 124: sine8=140; 125: sine8=137; 126: sine8=134; 127: sine8=131;
        
        128: sine8=128; 129: sine8=124; 130: sine8=121; 131: sine8=118; 132: sine8=115; 133: sine8=112; 134: sine8=109; 135: sine8=106;
        136: sine8=103; 137: sine8=100; 138: sine8=097; 139: sine8=093; 140: sine8=090; 141: sine8=088; 142: sine8=085; 143: sine8=082;
        144: sine8=079; 145: sine8=076; 146: sine8=073; 147: sine8=070; 148: sine8=067; 149: sine8=065; 150: sine8=062; 151: sine8=059;
        152: sine8=057; 153: sine8=054; 154: sine8=052; 155: sine8=049; 156: sine8=047; 157: sine8=044; 158: sine8=042; 159: sine8=040;
        160: sine8=037; 161: sine8=035; 162: sine8=033; 163: sine8=031; 164: sine8=029; 165: sine8=027; 166: sine8=025; 167: sine8=023;
        168: sine8=021; 169: sine8=020; 170: sine8=018; 171: sine8=017; 172: sine8=015; 173: sine8=014; 174: sine8=012; 175: sine8=011;
        176: sine8=010; 177: sine8=009; 178: sine8=007; 179: sine8=006; 180: sine8=005; 181: sine8=005; 182: sine8=004; 183: sine8=003;
        184: sine8=002; 185: sine8=002; 186: sine8=001; 187: sine8=001; 188: sine8=001; 189: sine8=000; 190: sine8=000; 191: sine8=000;
        192: sine8=000; 193: sine8=000; 194: sine8=000; 195: sine8=000; 196: sine8=001; 197: sine8=001; 198: sine8=001; 199: sine8=002;
        200: sine8=002; 201: sine8=003; 202: sine8=004; 203: sine8=005; 204: sine8=005; 205: sine8=006; 206: sine8=007; 207: sine8=009;
        208: sine8=010; 209: sine8=011; 210: sine8=012; 211: sine8=014; 212: sine8=015; 213: sine8=017; 214: sine8=018; 215: sine8=020;
        216: sine8=021; 217: sine8=023; 218: sine8=025; 219: sine8=027; 220: sine8=029; 221: sine8=031; 222: sine8=033; 223: sine8=035;
        224: sine8=037; 225: sine8=040; 226: sine8=042; 227: sine8=044; 228: sine8=047; 229: sine8=049; 230: sine8=052; 231: sine8=054;
        232: sine8=057; 233: sine8=059; 234: sine8=062; 235: sine8=065; 236: sine8=067; 237: sine8=070; 238: sine8=073; 239: sine8=076;
        240: sine8=079; 241: sine8=082; 242: sine8=085; 243: sine8=088; 244: sine8=090; 245: sine8=093; 246: sine8=097; 247: sine8=100;
        248: sine8=103; 249: sine8=106; 250: sine8=109; 251: sine8=112; 252: sine8=115; 253: sine8=118; 254: sine8=121; 255: sine8=124; 
	    default: sine8=255;
	 endcase
	end
   endfunction


//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line

//====================================================================


/*
generate
    
    if(TableSize==3'd3) // 8bit  
        assign outp = sine3( inp );		   
    else
    if(TableSize==3'd4) // 16bit  
        assign outp = sine4( inp );		   
    else
    if(TableSize==3'd5) // 32bit
        assign outp = sine5( inp );		
    else
    if(TableSize==3'd6) // 64bit 
        assign outp = sine6( inp );		
    else
    if(TableSize==3'd7) // 128bit
        assign outp = sine7( inp );		
    else                // 256bit
        assign outp = sine8( inp );		

    endgenerate
*/

/*

    assign outp = sine( inp );		// convert input code into sine wave
    
    generate
    if(TableBitDepth==3'd2)  // 16bit
    begin  
        function [7:0] sine2;			// sine wave function
        input   [3:0] phase;
 	    begin
	        case( phase )
              //16 steps, zero=127.5, half-wave amplitude 127.5
		      0: sine2=128;  1: sine2=176;  2: sine2=218;  3: sine2=245;  4: sine2=255;  5: sine2=245;  6: sine2=218;  7: sine2=176;
              8: sine2=128;  9: sine2=079; 10: sine2=037; 11: sine2=010; 12: sine2=000; 13: sine2=010; 14: sine2=037; 15: sine2=079;
	        default: sine2=255;
	        endcase
        end
        endfunction
    end
    endgenerate
*/

