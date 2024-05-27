//
// Copyright 2011 Ettus Research LLC
// Copyright 2018 Ettus Research, a National Instruments Company
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//



module add2
  #(parameter WIDTH=16)
    (input [WIDTH-1:0] in1,
     input [WIDTH-1:0] in2,
     output [WIDTH-1:0] sum);

   wire [WIDTH:0] 	sum_int = {in1[WIDTH-1],in1} + {in2[WIDTH-1],in2};
   assign 		sum = sum_int[WIDTH:1];  // Note -- will have some bias
   
endmodule // add2
