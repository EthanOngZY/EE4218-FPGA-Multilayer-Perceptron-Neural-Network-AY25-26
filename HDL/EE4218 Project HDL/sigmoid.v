`timescale 1ns / 1ps
/*
Sigmoid function implemented as a LUT, with input being the index and output being the value
*/

module sigmoid_lut 
	#(
		parameter width = 8,
		parameter inter_depth_bits = 7,
		parameter A_rows = 64,
		parameter hid_cols = 2
	)
	(
		input 	clk,
		input	Start_Sigmoid,
		output 	reg Done_Sigmoid,
		
		output	reg 	inter_read_en,
		output	reg 	[inter_depth_bits-1:0] inter_read_address,
		input	wire  	[width-1:0] inter_read_data_out,
		
		output 	reg 	inter2_write_en,
		output 	reg 	[inter_depth_bits-1:0] inter2_write_address,
		output 	reg 	[width-1:0] inter2_write_data_in,
		
		input [7:0] 	bias_C
	);
	
	// States
	localparam SigIdle 		= 5'b10000;
	localparam SigRead 		= 5'b01000;
	localparam SigWait 		= 5'b00100;
	localparam SigWrite 	= 5'b00010;
	localparam SigDone 		= 5'b00001;
	reg[4:0] State;

    // Counters
    reg [$clog2(2**inter_depth_bits):0] read_inter_counter 	= 0;
 	reg [$clog2(2**inter_depth_bits):0] write_inter_counter = 0;
 	
 	// LUT
	reg [7:0] lut [0:255];
    initial
    begin
    
    lut[0] = 8'd12;
    lut[1] = 8'd12;
    lut[2] = 8'd12;
    lut[3] = 8'd12;
    lut[4] = 8'd13;
    lut[5] = 8'd13;
    lut[6] = 8'd13;
    lut[7] = 8'd14;
    lut[8] = 8'd14;
    lut[9] = 8'd14;
    lut[10] = 8'd15;
    lut[11] = 8'd15;
    lut[12] = 8'd15;
    lut[13] = 8'd16;
    lut[14] = 8'd16;
    lut[15] = 8'd16;
    lut[16] = 8'd17;
    lut[17] = 8'd17;
    lut[18] = 8'd18;
    lut[19] = 8'd18;
    lut[20] = 8'd18;
    lut[21] = 8'd19;
    lut[22] = 8'd19;
    lut[23] = 8'd20;
    lut[24] = 8'd20;
    lut[25] = 8'd21;
    lut[26] = 8'd21;
    lut[27] = 8'd21;
    lut[28] = 8'd22;
    lut[29] = 8'd22;
    lut[30] = 8'd23;
    lut[31] = 8'd23;
    lut[32] = 8'd24;
    lut[33] = 8'd24;
    lut[34] = 8'd25;
    lut[35] = 8'd26;
    lut[36] = 8'd26;
    lut[37] = 8'd27;
    lut[38] = 8'd27;
    lut[39] = 8'd28;
    lut[40] = 8'd28;
    lut[41] = 8'd29;
    lut[42] = 8'd30;
    lut[43] = 8'd30;
    lut[44] = 8'd31;
    lut[45] = 8'd32;
    lut[46] = 8'd32;
    lut[47] = 8'd33;
    lut[48] = 8'd34;
    lut[49] = 8'd34;
    lut[50] = 8'd35;
    lut[51] = 8'd36;
    lut[52] = 8'd36;
    lut[53] = 8'd37;
    lut[54] = 8'd38;
    lut[55] = 8'd39;
    lut[56] = 8'd39;
    lut[57] = 8'd40;
    lut[58] = 8'd41;
    lut[59] = 8'd42;
    lut[60] = 8'd43;
    lut[61] = 8'd44;
    lut[62] = 8'd44;
    lut[63] = 8'd45;
    lut[64] = 8'd46;
    lut[65] = 8'd47;
    lut[66] = 8'd48;
    lut[67] = 8'd49;
    lut[68] = 8'd50;
    lut[69] = 8'd51;
    lut[70] = 8'd52;
    lut[71] = 8'd53;
    lut[72] = 8'd54;
    lut[73] = 8'd55;
    lut[74] = 8'd56;
    lut[75] = 8'd57;
    lut[76] = 8'd58;
    lut[77] = 8'd59;
    lut[78] = 8'd60;
    lut[79] = 8'd61;
    lut[80] = 8'd62;
    lut[81] = 8'd63;
    lut[82] = 8'd64;
    lut[83] = 8'd66;
    lut[84] = 8'd67;
    lut[85] = 8'd68;
    lut[86] = 8'd69;
    lut[87] = 8'd70;
    lut[88] = 8'd72;
    lut[89] = 8'd73;
    lut[90] = 8'd74;
    lut[91] = 8'd75;
    lut[92] = 8'd76;
    lut[93] = 8'd78;
    lut[94] = 8'd79;
    lut[95] = 8'd80;
    lut[96] = 8'd82;
    lut[97] = 8'd83;
    lut[98] = 8'd84;
    lut[99] = 8'd86;
    lut[100] = 8'd87;
    lut[101] = 8'd88;
    lut[102] = 8'd90;
    lut[103] = 8'd91;
    lut[104] = 8'd92;
    lut[105] = 8'd94;
    lut[106] = 8'd95;
    lut[107] = 8'd97;
    lut[108] = 8'd98;
    lut[109] = 8'd99;
    lut[110] = 8'd101;
    lut[111] = 8'd102;
    lut[112] = 8'd104;
    lut[113] = 8'd105;
    lut[114] = 8'd107;
    lut[115] = 8'd108;
    lut[116] = 8'd110;
    lut[117] = 8'd111;
    lut[118] = 8'd113;
    lut[119] = 8'd114;
    lut[120] = 8'd116;
    lut[121] = 8'd117;
    lut[122] = 8'd119;
    lut[123] = 8'd120;
    lut[124] = 8'd122;
    lut[125] = 8'd123;
    lut[126] = 8'd125;
    lut[127] = 8'd126;
    lut[128] = 8'd128;
    lut[129] = 8'd129;
    lut[130] = 8'd130;
    lut[131] = 8'd132;
    lut[132] = 8'd133;
    lut[133] = 8'd135;
    lut[134] = 8'd136;
    lut[135] = 8'd138;
    lut[136] = 8'd139;
    lut[137] = 8'd141;
    lut[138] = 8'd142;
    lut[139] = 8'd144;
    lut[140] = 8'd145;
    lut[141] = 8'd147;
    lut[142] = 8'd148;
    lut[143] = 8'd150;
    lut[144] = 8'd151;
    lut[145] = 8'd153;
    lut[146] = 8'd154;
    lut[147] = 8'd156;
    lut[148] = 8'd157;
    lut[149] = 8'd158;
    lut[150] = 8'd160;
    lut[151] = 8'd161;
    lut[152] = 8'd163;
    lut[153] = 8'd164;
    lut[154] = 8'd165;
    lut[155] = 8'd167;
    lut[156] = 8'd168;
    lut[157] = 8'd169;
    lut[158] = 8'd171;
    lut[159] = 8'd172;
    lut[160] = 8'd173;
    lut[161] = 8'd175;
    lut[162] = 8'd176;
    lut[163] = 8'd177;
    lut[164] = 8'd179;
    lut[165] = 8'd180;
    lut[166] = 8'd181;
    lut[167] = 8'd182;
    lut[168] = 8'd183;
    lut[169] = 8'd185;
    lut[170] = 8'd186;
    lut[171] = 8'd187;
    lut[172] = 8'd188;
    lut[173] = 8'd189;
    lut[174] = 8'd191;
    lut[175] = 8'd192;
    lut[176] = 8'd193;
    lut[177] = 8'd194;
    lut[178] = 8'd195;
    lut[179] = 8'd196;
    lut[180] = 8'd197;
    lut[181] = 8'd198;
    lut[182] = 8'd199;
    lut[183] = 8'd200;
    lut[184] = 8'd201;
    lut[185] = 8'd202;
    lut[186] = 8'd203;
    lut[187] = 8'd204;
    lut[188] = 8'd205;
    lut[189] = 8'd206;
    lut[190] = 8'd207;
    lut[191] = 8'd208;
    lut[192] = 8'd209;
    lut[193] = 8'd210;
    lut[194] = 8'd211;
    lut[195] = 8'd211;
    lut[196] = 8'd212;
    lut[197] = 8'd213;
    lut[198] = 8'd214;
    lut[199] = 8'd215;
    lut[200] = 8'd216;
    lut[201] = 8'd216;
    lut[202] = 8'd217;
    lut[203] = 8'd218;
    lut[204] = 8'd219;
    lut[205] = 8'd219;
    lut[206] = 8'd220;
    lut[207] = 8'd221;
    lut[208] = 8'd221;
    lut[209] = 8'd222;
    lut[210] = 8'd223;
    lut[211] = 8'd223;
    lut[212] = 8'd224;
    lut[213] = 8'd225;
    lut[214] = 8'd225;
    lut[215] = 8'd226;
    lut[216] = 8'd227;
    lut[217] = 8'd227;
    lut[218] = 8'd228;
    lut[219] = 8'd228;
    lut[220] = 8'd229;
    lut[221] = 8'd229;
    lut[222] = 8'd230;
    lut[223] = 8'd231;
    lut[224] = 8'd231;
    lut[225] = 8'd232;
    lut[226] = 8'd232;
    lut[227] = 8'd233;
    lut[228] = 8'd233;
    lut[229] = 8'd234;
    lut[230] = 8'd234;
    lut[231] = 8'd234;
    lut[232] = 8'd235;
    lut[233] = 8'd235;
    lut[234] = 8'd236;
    lut[235] = 8'd236;
    lut[236] = 8'd237;
    lut[237] = 8'd237;
    lut[238] = 8'd237;
    lut[239] = 8'd238;
    lut[240] = 8'd238;
    lut[241] = 8'd239;
    lut[242] = 8'd239;
    lut[243] = 8'd239;
    lut[244] = 8'd240;
    lut[245] = 8'd240;
    lut[246] = 8'd240;
    lut[247] = 8'd241;
    lut[248] = 8'd241;
    lut[249] = 8'd241;
    lut[250] = 8'd242;
    lut[251] = 8'd242;
    lut[252] = 8'd242;
    lut[253] = 8'd243;
    lut[254] = 8'd243;
    lut[255] = 8'd243;
	end

    always @(posedge clk)
    begin
        case(State)
        	
        	SigIdle:
        	begin
        		Done_Sigmoid 			<= 0;
        		read_inter_counter 		<= 0;
        		write_inter_counter 	<= 0;
        		
        		inter_read_en 			<= 0;
        		inter_read_address 		<= 0;
        		inter2_write_en 		<= 0;
        		inter2_write_address 	<= 0;
        		inter2_write_data_in 	<= 0;
        		
        		if (Start_Sigmoid)
        		begin
        			State 				<= SigRead;
        		end
        	end
        	
        	SigRead:
        	begin
        		inter_read_en 			<= 1;
        		inter_read_address 		<= read_inter_counter;
        		State 					<= SigWait;
        	end
        	
        	SigWait:
        	begin
        		State 					<= SigWrite;
        	end
        	
        	SigWrite:
        	begin
        	    inter2_write_en 		<= 1;
        		inter2_write_data_in 	<= lut[inter_read_data_out];
        		inter2_write_address 	<= read_inter_counter;
        		
        		if (read_inter_counter == (A_rows*hid_cols)-1)
        		begin
        			State 				<= SigDone;
        		end
        		else
        		begin
        			read_inter_counter 	<= read_inter_counter + 1;
        			State 				<= SigRead;
        		end
        	end
        	
        	SigDone:
        	begin
        		inter2_write_en			<= 0;
        		inter_read_en 			<= 0;
        		Done_Sigmoid 			<= 1;
        		
        		if (!Start_Sigmoid)
        			State 				<= SigIdle;
        	end
        	
        	default: State 				<= SigIdle;
        endcase
    end
endmodule
