`timescale 1ns / 1ps

module myip_v1_0
(
    ACLK,
	ARESETN,
	S_AXIS_TREADY,
	S_AXIS_TDATA,
	S_AXIS_TLAST,
	S_AXIS_TVALID,
	M_AXIS_TVALID,
	M_AXIS_TDATA,
	M_AXIS_TLAST,
	M_AXIS_TREADY
);

	input          		ACLK;
    input          		ARESETN;
	// slave in interface
    output reg	   		S_AXIS_TREADY;
    input  [31:0] 		S_AXIS_TDATA;
    input          		S_AXIS_TLAST;
    input          		S_AXIS_TVALID;
	// master out interface
    output reg         	M_AXIS_TVALID;
    output reg  [31:0] 	M_AXIS_TDATA;
    output reg         	M_AXIS_TLAST;
    input			   	M_AXIS_TREADY;
	
	// RAM Parameters
	localparam A_depth_bits		= 9; // inputs
	localparam hid_depth_bits	= 3; // hidden layers
	localparam out_depth_bits	= 2; // output layer
	localparam inter_depth_bits	= 7; // intermediate results
	localparam RES_depth_bits	= 6; // final results
	localparam width			= 8;
	
	// Matrix Parameters
	localparam A_rows			= 64;
	localparam A_cols			= 7;
	localparam hid_rows			= 8;
	localparam hid_cols			= 2;
	localparam out_rows			= 3;
	localparam out_cols			= 1;
	
	// Matrix Sizes
    localparam A_words					= A_rows*A_cols;
    localparam hid_words				= hid_rows*hid_cols;
    localparam out_words				= out_rows*out_cols;
    localparam inter_words				= A_rows*hid_cols;
    localparam RES_words				= A_rows*out_cols;
	localparam NUMBER_OF_INPUT_WORDS	= A_words+hid_words+out_words;
	
	// RAM Stuff
	// Input read paths
    reg                    				A_write_en;
    reg  	[A_depth_bits-1:0]			A_write_address;
    reg	 	[width-1:0]        			A_write_data_in;
    wire                    			A_read_en;
    wire 	[A_depth_bits-1:0] 			A_read_address;
    wire 	[width-1:0]        			A_read_data_out;

	// intermediate read paths
	wire                    			inter_write_en;
    wire  	[A_depth_bits-1:0]			inter_write_address;
    wire	[width-1:0]        			inter_write_data_in;
    wire                    			inter_read_en;
    wire 	[A_depth_bits-1:0] 			inter_read_address;
    wire 	[width-1:0]        			inter_read_data_out;
    
    //intermediate 2 read paths
    wire                    			inter2_write_en;
    wire  	[A_depth_bits-1:0]			inter2_write_address;
    wire	[width-1:0]        			inter2_write_data_in;
    wire                    			inter2_read_en;
    wire 	[A_depth_bits-1:0] 			inter2_read_address;
    wire 	[width-1:0]        			inter2_read_data_out;
    
    // hidden weights column 1 read paths
    reg                    				hid1_write_en;
    reg 	[hid_depth_bits-1:0] 		hid1_write_address;
    reg  	[width-1:0]              	hid1_write_data_in;
    wire                    			hid1_read_en;
    wire 	[hid_depth_bits-1:0] 		hid1_read_address;
    wire  	[width-1:0]              	hid1_read_data_out;
    
    // hidden weights column 2 read paths
    reg                    				hid2_write_en;
    reg 	[hid_depth_bits-1:0] 		hid2_write_address;
    reg  	[width-1:0]              	hid2_write_data_in;
    wire                    			hid2_read_en;
    wire	[hid_depth_bits-1:0] 		hid2_read_address;
    wire  	[width-1:0]              	hid2_read_data_out;
    
    // output weights read paths
    reg                    				out_write_en;
    reg 	[hid_depth_bits-1:0] 		out_write_address;
    reg  	[width-1:0]              	out_write_data_in;
    wire                    			out_read_en;
    wire 	[hid_depth_bits-1:0] 		out_read_address;
    wire  	[width-1:0]              	out_read_data_out;
    
    //RES read paths
    wire                    			RES_write_en;
    wire  	[RES_depth_bits-1:0] 		RES_write_address;
    wire 	[width-1:0]              	RES_write_data_out;
    reg                     			RES_read_en;
    reg	 	[RES_depth_bits-1:0] 		RES_read_address;
    wire 	[width-1:0]              	RES_read_data_in;

    // Wires to flag sub-modules
    reg 	Hidden_Start;
    wire 	Hidden_Done;
    reg		Start_Sigmoid;
    wire 	Done_Sigmoid;
    reg 	Start_Output;
    wire 	Done_Output;

	// State Parameters
    localparam Idle  			= 7'b1000000;
	localparam Read_Inputs 		= 7'b0100000;
	localparam ComputeHid 		= 7'b0010000;
	localparam Sigmoid			= 7'b0001000;
	localparam Wait 			= 7'b0000100;
	localparam Output_Layer		= 7'b0000010;
	localparam Write_Outputs  	= 7'b0000001;
	reg [6:0]  State;
	
	// Counters
	reg [$clog2(NUMBER_OF_INPUT_WORDS) - 1:0] 	read_counter;
	reg [$clog2(RES_words) - 1:0] 				write_counter;
	
	// Bias as Varaibles
	reg [$clog2(256):0] bias_A; //first bias of w_hid
	reg [$clog2(256):0] bias_B; //second bias of w_hid
	reg [$clog2(256):0] bias_C; //first bias of w_out
	
	// Flags for writing out results
	reg Write_wait; 
	reg Write_start;
	reg Write_captured;
	reg [7:0] res_buf;

	always @(posedge ACLK)
	begin
		if (!ARESETN)
		begin
			// CAUTION: make sure your reset polarity is consistent with the system reset polarity
			State	<= Idle;
        end
        else
        begin
		case(State)
			Idle:
			begin
				// Resetting various variables
				Hidden_Start 		<= 0;
				Start_Sigmoid		<= 0;
				Start_Output		<= 0;
				read_counter 		<= 0;
				write_counter 		<= 0;
				
				A_write_en 			<= 0;
				A_write_address 	<= 0;
				hid1_write_en 		<= 0;
				hid1_write_address 	<= 0;
				hid2_write_en 		<= 0;
				hid2_write_address 	<= 0;
				out_write_en 		<= 0;
				out_write_address 	<= 0;
				
				// resetting variables
				bias_A				<= 0;
				bias_B				<= 0;
				bias_C				<= 0;
				
				// Resetting AXI signals
				S_AXIS_TREADY 		<= 0;
				M_AXIS_TVALID		<= 0;
				M_AXIS_TLAST		<= 0;
				
				// Resetting write flags
				Write_start 		<= 1'b0;
           		Write_wait    		<= 1'b0;
            	Write_captured 		<= 1'b0;
            	res_buf				<= 8'b0;
				
				if (S_AXIS_TVALID == 1)
				begin
					S_AXIS_TREADY 		<= 1;
					hid1_write_en		<= 1;
					hid1_write_address 	<= read_counter;
					hid1_write_data_in 	<= S_AXIS_TDATA;
			
					State       		<= Read_Inputs;
					// start receiving data once you go into Read_Inputs
				end
			end
			
			Read_Inputs:
			begin
				S_AXIS_TREADY 				<= 1;
				if (S_AXIS_TVALID == 1)
				begin
					if (read_counter < hid_rows) // hidden wrights column 1
					begin
						hid1_write_en 		<= 1;
						hid1_write_address 	<= read_counter;
						hid1_write_data_in 	<= S_AXIS_TDATA;
						// saving bias as variable
						if (read_counter == hid_rows-1)
							bias_A 			<= S_AXIS_TDATA;
					end
					else if (read_counter < hid_words) // hidden wrights column 2
					begin
						hid1_write_en 		<= 0;
						hid2_write_en 		<= 1;
						hid2_write_address 	<= (read_counter - hid_rows);
						hid2_write_data_in 	<= S_AXIS_TDATA;
						// saving bias as variable
						if (read_counter == hid_words-1)
							bias_B 			<= S_AXIS_TDATA;
					end
					else if (read_counter < hid_words + out_words) // output weights
					begin
						hid2_write_en 		<= 0;
						out_write_en 		<= 1;
						out_write_address 	<= (read_counter - hid_words);
						out_write_data_in 	<= S_AXIS_TDATA;
						// saving bias as variable
						if (read_counter == (hid_words + out_words - 1))
							bias_C 			<= S_AXIS_TDATA;
					end
					else if (read_counter < NUMBER_OF_INPUT_WORDS) // input matrix
					begin
						out_write_en 		<= 0;
						A_write_en 			<= 1;
						A_write_address 	<= (read_counter - hid_words - out_words);
						A_write_data_in 	<= S_AXIS_TDATA;
					end
					
					if (read_counter == NUMBER_OF_INPUT_WORDS-1)
					begin
						A_write_en <= 1;
						A_write_address 	<= (read_counter - hid_words - out_words);
						A_write_data_in 	<= S_AXIS_TDATA;
						
						hid1_write_en 		<= 0;
						hid2_write_en 		<= 0;
						out_write_en 		<= 0;
						
						S_AXIS_TREADY 		<= 0;
						State 				<= ComputeHid;
					end
					
					read_counter 			<= read_counter + 1;
				end
			end
			
			ComputeHid:
			begin
				A_write_en 				<= 0;
				read_counter 			<= 0;
				hid1_write_address 		<= 0;
				hid2_write_address 		<= 0;
				out_write_address 		<= 0;
				A_write_address 		<= 0;
				
				Hidden_Start 			<= 1;
				S_AXIS_TREADY 			<= 0;
				
				if(Hidden_Done == 1)
				begin
					Hidden_Start 		<= 0;
					State 				<= Wait;
					
				end
			end
			
			Wait:
			begin
				write_counter 			<= 0;
				RES_read_en 			<= 0;
				RES_read_address 		<= 0;
				
				State 					<= Sigmoid;
			end
			
			Sigmoid:
			begin
				Start_Sigmoid <= 1;
				S_AXIS_TREADY <= 0;
				
				if (Done_Sigmoid == 1)
				begin
					Start_Sigmoid <= 0;
					State <= Output_Layer;
				end
			end
			
			Output_Layer:
			begin
				Start_Output <= 1;
				S_AXIS_TREADY <= 0;
				
				if (Done_Output == 1)
				begin
					Start_Output <= 0;
					Write_wait 			<= 0;
					State <= Write_Outputs;
				end
           	end
           	
           	Write_Outputs:
           	begin
            	S_AXIS_TREADY	<= 0;
					
					if (!Write_start)
					begin
						RES_read_en			<= 1;
						RES_read_address	<= write_counter;
						
						Write_start			<= 1;
						Write_wait			<= 1;
						Write_captured		<= 0;
					end
					
					else if (Write_wait)
					begin
						Write_wait	<= 0;
						Write_captured	<= 1;
                    end
                    
                    else if (Write_captured)
                    begin
                    	res_buf	<= RES_read_data_in;
                    	Write_captured	<= 0;
                    end
                    
                    else
                    begin
                    	if (!M_AXIS_TVALID)
    						M_AXIS_TVALID <= 1;
                    		M_AXIS_TDATA	<= res_buf;
                    		M_AXIS_TLAST	<= (write_counter == RES_words - 1);
                    	
                    	if (M_AXIS_TVALID && M_AXIS_TREADY)
                    	begin
                    		M_AXIS_TVALID <= 1'b0;
                            M_AXIS_TLAST  <= 1'b0;
                            Write_start	<= 0;
                            
                            if (write_counter == RES_words - 1)
                            begin
                            	RES_read_en	<= 0;
                            	State	<= Idle;
                            end
                            else begin
                            	write_counter	<= write_counter + 1;
                            end
                    	end
                    end
           	end
		endcase
	end
end
	
	// Connection to Sub-Modules
	memory_RAM 
	#(
        .width(width),
        .depth_bits(A_depth_bits)
    ) A_RAM (
        .clk(ACLK),
        .write_en(A_write_en),
        .write_address(A_write_address),
        .write_data_in(A_write_data_in),
        .read_en(A_read_en),
        .read_address(A_read_address),
        .read_data_out(A_read_data_out)
    );
    
    memory_RAM 
    #(
        .width(width),
        .depth_bits(inter_depth_bits)
    ) inter_RAM (
        .clk(ACLK),
        .write_en(inter_write_en),
        .write_address(inter_write_address),
        .write_data_in(inter_write_data_in),
        .read_en(inter_read_en),
        .read_address(inter_read_address),
        .read_data_out(inter_read_data_out)
    );
    
    memory_RAM 
    #(
        .width(width),
        .depth_bits(inter_depth_bits)
    ) inter2_RAM (
        .clk(ACLK),
        .write_en(inter2_write_en),
        .write_address(inter2_write_address),
        .write_data_in(inter2_write_data_in),
        .read_en(inter2_read_en),
        .read_address(inter2_read_address),
        .read_data_out(inter2_read_data_out)
    );
    
    memory_RAM 
	#(
		.width(width), 
		.depth_bits(hid_depth_bits)
	) hid1_RAM 
	(
		.clk(ACLK),
		.write_en(hid1_write_en),
		.write_address(hid1_write_address),
		.write_data_in(hid1_write_data_in),
		.read_en(hid1_read_en),    
		.read_address(hid1_read_address),
		.read_data_out(hid1_read_data_out)
	);
	
	memory_RAM 
	#(
		.width(width), 
		.depth_bits(hid_depth_bits)
	) hid2_RAM 
	(
		.clk(ACLK),
		.write_en(hid2_write_en),
		.write_address(hid2_write_address),
		.write_data_in(hid2_write_data_in),
		.read_en(hid2_read_en),    
		.read_address(hid2_read_address),
		.read_data_out(hid2_read_data_out)
	);
	
	memory_RAM 
	#(
		.width(width), 
		.depth_bits(out_depth_bits)
	) out_RAM 
	(
		.clk(ACLK),
		.write_en(out_write_en),
		.write_address(out_write_address),
		.write_data_in(out_write_data_in),
		.read_en(out_read_en),    
		.read_address(out_read_address),
		.read_data_out(out_read_data_out)
	);
	
	memory_RAM 
	#(
		.width(width), 
		.depth_bits(RES_depth_bits)
	) RES_RAM 
	(
		.clk(ACLK),
		.write_en(RES_write_en),
		.write_address(RES_write_address),
		.write_data_in(RES_write_data_out),
		.read_en(RES_read_en),    
		.read_address(RES_read_address),
		.read_data_out(RES_read_data_in)
	);
	
	hidden_layer
	#(
		.width(width),
		.A_depth_bits(A_depth_bits),
		.hid_depth_bits(hid_depth_bits),
		.inter_depth_bits(inter_depth_bits),
		.A_rows(A_rows),
		.A_cols(A_cols),
		.hid_rows(hid_rows),
		.hid_cols(hid_cols)
	) hidden_0
	(
		.clk(ACLK),
		.Hidden_Start(Hidden_Start),
		.Hidden_Done(Hidden_Done),
		
		.A_read_en(A_read_en),
		.A_read_address(A_read_address),
		.A_read_data_out(A_read_data_out),
		
		.hid1_read_en(hid1_read_en),
		.hid1_read_address(hid1_read_address),
		.hid1_read_data_out(hid1_read_data_out),
		
		.hid2_read_en(hid2_read_en),
		.hid2_read_address(hid2_read_address),
		.hid2_read_data_out(hid2_read_data_out),
		
		.inter_write_en(inter_write_en),
		.inter_write_address(inter_write_address),
		.inter_write_data_in(inter_write_data_in),
		
		.bias_A(bias_A),
		.bias_B(bias_B)
	);
	
	sigmoid_lut
	#(
		.width(width),
		.inter_depth_bits(inter_depth_bits),
		.A_rows(A_rows),
		.hid_cols(hid_cols)
	) sigmoid_0
	(
		.clk(ACLK),
		.Start_Sigmoid(Start_Sigmoid),
		.Done_Sigmoid(Done_Sigmoid),
		
		.inter_read_en(inter_read_en),
		.inter_read_address(inter_read_address),
		.inter_read_data_out(inter_read_data_out),
		
		.inter2_write_en(inter2_write_en),
		.inter2_write_address(inter2_write_address),
		.inter2_write_data_in(inter2_write_data_in)
	);
	
	output_layer
	#(
		.width(width),
		.out_depth_bits(out_depth_bits),
		.RES_depth_bits(RES_depth_bits),
		.inter_depth_bits(inter_depth_bits),
		.A_rows(A_rows),
		.hid_cols(hid_cols),
		.out_rows(out_rows),
		.out_cols(out_cols)
	) output_0
	(
		.clk(ACLK),
		.Start_Output(Start_Output),
		.Done_Output(Done_Output),
		
		.inter2_read_en(inter2_read_en),
		.inter2_read_address(inter2_read_address),
		.inter2_read_data_out(inter2_read_data_out),
		
		.out_read_en(out_read_en),
		.out_read_address(out_read_address),
		.out_read_data_out(out_read_data_out),
		
		.RES_write_en(RES_write_en),
		.RES_write_address(RES_write_address),
		.RES_write_data_out(RES_write_data_out),
		
		.bias_C(bias_C)
	);
endmodule