@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%s\00", align 1

define dso_local i32 @getint() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* %1)
  %3 = load i32, i32* %1, align 4
  ret i32 %3
}

declare dso_local i32 @__isoc99_scanf(i8*, ...) #1

define dso_local void @putint(i32 %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32 %3)
  ret void
}

declare dso_local i32 @printf(i8*, ...) #1

define dso_local void @putstr(i8* %0) #0 {
  %2 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0), i8* %3)
  ret void
}


@A_GLOBAL = constant i32 1
@B_GLOBAL = constant i32 2
@C_GLOBAL = constant i32 3
@D_GLOBAL = constant i32 4
@E_GLOBAL = constant i32 5
@F_GLOBAL = constant i32 6
@Arr1_1_GLOBAL = constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5]
@Arr2_1_GLOBAL = constant [6 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6]
@a_glogal = global i32 zeroinitializer
@arr1_1_global = global [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5]
@arr2_1_global = global [6 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6]

@.str1 = constant [17 x i8] c"Now a_global is \00"
@.str2 = constant [15 x i8] c", it is < 50.\0a\00"
@.str3 = constant [32 x i8] c"Now a_global is exactly 50!!!.\0a\00"
@.str4 = constant [17 x i8] c"Now a_global is \00"
@.str5 = constant [15 x i8] c", it is > 50.\0a\00"
@.str6 = constant [34 x i8] c"I am a function with 3 param: {{ \00"
@.str7 = constant [3 x i8] c", \00"
@.str8 = constant [3 x i8] c", \00"
@.str9 = constant [18 x i8] c" }} and I return \00"
@.str10 = constant [3 x i8] c".\0a\00"
@.str11 = constant [41 x i8] c"I am a function with only one param: {{ \00"
@.str12 = constant [26 x i8] c" }} and I return noting.\0a\00"
@.str13 = constant [34 x i8] c"I am a function with 2 param: {{ \00"
@.str14 = constant [3 x i8] c", \00"
@.str15 = constant [26 x i8] c" }} and I return noting.\0a\00"
@.str16 = constant [48 x i8] c"I am a function without params and I return 0.\0a\00"
@.str17 = constant [10 x i8] c"19241091\0a\00"
@.str18 = constant [12 x i8] c"A_GLOBAL = \00"
@.str19 = constant [2 x i8] c"\0a\00"
@.str20 = constant [24 x i8] c"arr2_1_global[1][1] is \00"
@.str21 = constant [2 x i8] c"\0a\00"
@.str22 = constant [19 x i8] c"Now main_var_c is \00"
@.str23 = constant [2 x i8] c"\0a\00"
@.str24 = constant [19 x i8] c"Now main_var_a is \00"
@.str25 = constant [15 x i8] c", it is >= 30\0a\00"
@.str26 = constant [19 x i8] c"Now main_var_a is \00"
@.str27 = constant [15 x i8] c", it is <= 10\0a\00"
@.str28 = constant [19 x i8] c"Now main_var_a is \00"
@.str29 = constant [23 x i8] c", it is > 10 and < 30\0a\00"
@.str30 = constant [50 x i8] c"==================testfile1 end==================\00"

define i32 @func_params(i32 %a, i32 * %b, i32 * %c) #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = alloca i32, align 4
	%x5 = getelementptr inbounds i32, i32* %t4, i32 0
	; store 6
	store i32 %a, i32* %x5, align 4
	%x7 = load i32, i32* %x5, align 4
	%x8 = mul i32 %x7, 2
	; getelementptr 9
	%x9 = getelementptr inbounds i32, i32* %b, i32 0
	%x10 = load i32, i32* %x9, align 4
	%x11 = srem i32 %x10, 2
	%x12 = add i32 %x8, %x11
	; getelementptr 13
	%x13 = getelementptr inbounds i32, i32* %c, i32 0
	; getelementptr 14
	%x14 = getelementptr inbounds i32, i32* %x13, i32 0
	%x15 = load i32, i32* %x14, align 4
	%x16 = srem i32 %x15, 3
	%x17 = add i32 %x12, %x16
	%t18 = alloca i32, align 4
	%x19 = getelementptr inbounds i32, i32* %t18, i32 0
	; store 20
	store i32 %x17, i32* %x19, align 4
	%x21 = load i32, i32* %x19, align 4
	%x22 = load i32, i32* %x5, align 4
	%x23 = add i32 %x22, 2
	%x24 = mul i32 %x21, %x23
	; store 25
	store i32 %x24, i32* @a_glogal, align 4
	%x26 = load i32, i32* @a_glogal, align 4
	%t27 = icmp slt i32 %x26, 50
	%x28 = zext i1 %t27 to i32
	; if %x28 then b1 else b2
	%t29 = icmp ne i32 %x28, 0
	br i1 %t29, label %b1, label %b2
b1: ; preds = %b0
	%x30 = load i32, i32* @a_glogal, align 4
	%t31 = getelementptr inbounds [17 x i8], [17 x i8]* @.str1, i32 0, i32 0
	call void @putstr(i8* %t31)
	call void @putint(i32 %x30)
	%t32 = getelementptr inbounds [15 x i8], [15 x i8]* @.str2, i32 0, i32 0
	call void @putstr(i8* %t32)
	br label %b6
b2: ; preds = %b0
	%x33 = load i32, i32* @a_glogal, align 4
	%t34 = icmp eq i32 %x33, 50
	%x35 = zext i1 %t34 to i32
	; if %x35 then b3 else b4
	%t36 = icmp ne i32 %x35, 0
	br i1 %t36, label %b3, label %b4
b3: ; preds = %b2
	%t37 = getelementptr inbounds [32 x i8], [32 x i8]* @.str3, i32 0, i32 0
	call void @putstr(i8* %t37)
	br label %b5
b4: ; preds = %b2
	%x38 = load i32, i32* @a_glogal, align 4
	%t39 = getelementptr inbounds [17 x i8], [17 x i8]* @.str4, i32 0, i32 0
	call void @putstr(i8* %t39)
	call void @putint(i32 %x38)
	%t40 = getelementptr inbounds [15 x i8], [15 x i8]* @.str5, i32 0, i32 0
	call void @putstr(i8* %t40)
	br label %b5
b5: ; preds = %b3, %b4
	br label %b6
b6: ; preds = %b1, %b5
	ret i32 0
}

define i32 @func2_params(i32 %a, i32 %b, i32 %c) #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = alloca i32, align 4
	%x5 = getelementptr inbounds i32, i32* %t4, i32 0
	; store 6
	store i32 %a, i32* %x5, align 4
	%t7 = alloca i32, align 4
	%x8 = getelementptr inbounds i32, i32* %t7, i32 0
	; store 9
	store i32 %b, i32* %x8, align 4
	%t10 = alloca i32, align 4
	%x11 = getelementptr inbounds i32, i32* %t10, i32 0
	; store 12
	store i32 %c, i32* %x11, align 4
	%x13 = load i32, i32* %x5, align 4
	%x14 = load i32, i32* %x8, align 4
	%x15 = add i32 %x13, %x14
	%x16 = load i32, i32* %x11, align 4
	%x17 = add i32 %x15, %x16
	%x18 = mul i32 6, %x17
	%x19 = sdiv i32 %x18, 2
	%x20 = sub i32 %x19, 2
	%t21 = alloca i32, align 4
	%x22 = getelementptr inbounds i32, i32* %t21, i32 0
	; store 23
	store i32 %x20, i32* %x22, align 4
	%x24 = load i32, i32* %x5, align 4
	%x25 = load i32, i32* %x8, align 4
	%x26 = load i32, i32* %x11, align 4
	%x27 = load i32, i32* %x22, align 4
	%t28 = getelementptr inbounds [34 x i8], [34 x i8]* @.str6, i32 0, i32 0
	call void @putstr(i8* %t28)
	call void @putint(i32 %x24)
	%t29 = getelementptr inbounds [3 x i8], [3 x i8]* @.str7, i32 0, i32 0
	call void @putstr(i8* %t29)
	call void @putint(i32 %x25)
	%t30 = getelementptr inbounds [3 x i8], [3 x i8]* @.str8, i32 0, i32 0
	call void @putstr(i8* %t30)
	call void @putint(i32 %x26)
	%t31 = getelementptr inbounds [18 x i8], [18 x i8]* @.str9, i32 0, i32 0
	call void @putstr(i8* %t31)
	call void @putint(i32 %x27)
	%t32 = getelementptr inbounds [3 x i8], [3 x i8]* @.str10, i32 0, i32 0
	call void @putstr(i8* %t32)
	%x33 = load i32, i32* %x22, align 4
	ret i32 %x33
}

define void @func3_params(i32 %a) #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = alloca i32, align 4
	%x5 = getelementptr inbounds i32, i32* %t4, i32 0
	; store 6
	store i32 %a, i32* %x5, align 4
	%x7 = load i32, i32* %x5, align 4
	%t8 = getelementptr inbounds [41 x i8], [41 x i8]* @.str11, i32 0, i32 0
	call void @putstr(i8* %t8)
	call void @putint(i32 %x7)
	%t9 = getelementptr inbounds [26 x i8], [26 x i8]* @.str12, i32 0, i32 0
	call void @putstr(i8* %t9)
	ret void
}

define void @func4_params(i32 %a, i32 %b) #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = alloca i32, align 4
	%x5 = getelementptr inbounds i32, i32* %t4, i32 0
	; store 6
	store i32 %a, i32* %x5, align 4
	%t7 = alloca i32, align 4
	%x8 = getelementptr inbounds i32, i32* %t7, i32 0
	; store 9
	store i32 %b, i32* %x8, align 4
	%x10 = load i32, i32* %x5, align 4
	%x11 = load i32, i32* %x8, align 4
	%t12 = getelementptr inbounds [34 x i8], [34 x i8]* @.str13, i32 0, i32 0
	call void @putstr(i8* %t12)
	call void @putint(i32 %x10)
	%t13 = getelementptr inbounds [3 x i8], [3 x i8]* @.str14, i32 0, i32 0
	call void @putstr(i8* %t13)
	call void @putint(i32 %x11)
	%t14 = getelementptr inbounds [26 x i8], [26 x i8]* @.str15, i32 0, i32 0
	call void @putstr(i8* %t14)
	ret void
}

define i32 @func_no_params() #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = getelementptr inbounds [48 x i8], [48 x i8]* @.str16, i32 0, i32 0
	call void @putstr(i8* %t4)
	ret i32 0
}

define void @func2_no_params() #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	ret void
}

define i32 @main() #0 {
b0: ; preds = 
	; getelementptr 0
	%x0 = getelementptr inbounds [5 x i32], [5 x i32]* @Arr1_1_GLOBAL, i32 0, i32 0
	; getelementptr 1
	%x1 = getelementptr inbounds [6 x i32], [6 x i32]* @Arr2_1_GLOBAL, i32 0, i32 0
	; getelementptr 2
	%x2 = getelementptr inbounds [5 x i32], [5 x i32]* @arr1_1_global, i32 0, i32 0
	; getelementptr 3
	%x3 = getelementptr inbounds [6 x i32], [6 x i32]* @arr2_1_global, i32 0, i32 0
	%t4 = alloca i32, align 4
	%x5 = getelementptr inbounds i32, i32* %t4, i32 0
	%t6 = alloca i32, align 4
	%x7 = getelementptr inbounds i32, i32* %t6, i32 0
	%t8 = alloca i32, align 4
	%x9 = getelementptr inbounds i32, i32* %t8, i32 0
	%x10 = call i32 @getint()
	; store 11
	store i32 %x10, i32* %x5, align 4
	%x12 = call i32 @getint()
	; store 13
	store i32 %x12, i32* %x7, align 4
	%t14 = getelementptr inbounds [10 x i8], [10 x i8]* @.str17, i32 0, i32 0
	call void @putstr(i8* %t14)
	%t15 = getelementptr inbounds [12 x i8], [12 x i8]* @.str18, i32 0, i32 0
	call void @putstr(i8* %t15)
	call void @putint(i32 1)
	%t16 = getelementptr inbounds [2 x i8], [2 x i8]* @.str19, i32 0, i32 0
	call void @putstr(i8* %t16)
	call void @func3_params(i32 3)
	call void @func4_params(i32 1, i32 2)
	%x17 = call i32 @func_no_params()
	call void @func2_no_params()
	; getelementptr 18
	%x18 = getelementptr inbounds i32, i32* %x2, i32 1
	; store 19
	store i32 2, i32* %x18, align 4
	; getelementptr 20
	%x20 = getelementptr inbounds i32, i32* %x2, i32 4
	; store 21
	store i32 3, i32* %x20, align 4
	; getelementptr 22
	%x22 = getelementptr inbounds i32, i32* %x2, i32 2
	; store 23
	store i32 4, i32* %x22, align 4
	; getelementptr 24
	%x24 = getelementptr inbounds i32, i32* %x3, i32 2
	; getelementptr 25
	%x25 = getelementptr inbounds i32, i32* %x24, i32 1
	; store 26
	store i32 4, i32* %x25, align 4
	; getelementptr 27
	%x27 = getelementptr inbounds i32, i32* %x3, i32 2
	; getelementptr 28
	%x28 = getelementptr inbounds i32, i32* %x27, i32 1
	; getelementptr 29
	%x29 = getelementptr inbounds i32, i32* %x2, i32 1
	%x30 = load i32, i32* %x29, align 4
	%x31 = mul i32 %x30, 2
	; store 32
	store i32 %x31, i32* %x28, align 4
	; getelementptr 33
	%x33 = getelementptr inbounds i32, i32* %x3, i32 2
	; getelementptr 34
	%x34 = getelementptr inbounds i32, i32* %x33, i32 1
	%x35 = load i32, i32* %x34, align 4
	%t36 = getelementptr inbounds [24 x i8], [24 x i8]* @.str20, i32 0, i32 0
	call void @putstr(i8* %t36)
	call void @putint(i32 %x35)
	%t37 = getelementptr inbounds [2 x i8], [2 x i8]* @.str21, i32 0, i32 0
	call void @putstr(i8* %t37)
	; getelementptr 38
	%x38 = getelementptr inbounds i32, i32* %x2, i32 0
	; getelementptr 39
	%x39 = getelementptr inbounds i32, i32* %x3, i32 0
	%x40 = call i32 @func_params(i32 1, i32 * %x38, i32 * %x39)
	; getelementptr 41
	%x41 = getelementptr inbounds i32, i32* %x2, i32 1
	%x42 = load i32, i32* %x41, align 4
	; getelementptr 43
	%x43 = getelementptr inbounds i32, i32* %x3, i32 2
	; getelementptr 44
	%x44 = getelementptr inbounds i32, i32* %x43, i32 1
	%x45 = load i32, i32* %x44, align 4
	%x46 = call i32 @func2_params(i32 2, i32 %x42, i32 %x45)
	; store 47
	store i32 %x46, i32* %x9, align 4
	%x48 = load i32, i32* %x9, align 4
	%t49 = getelementptr inbounds [19 x i8], [19 x i8]* @.str22, i32 0, i32 0
	call void @putstr(i8* %t49)
	call void @putint(i32 %x48)
	%t50 = getelementptr inbounds [2 x i8], [2 x i8]* @.str23, i32 0, i32 0
	call void @putstr(i8* %t50)
	%x51 = load i32, i32* %x5, align 4
	%x52 = srem i32 %x51, 7
	%t53 = icmp ne i32 %x52, 0
	%x54 = zext i1 %t53 to i32
	; if %x54 then b1 else b2
	%t55 = icmp ne i32 %x54, 0
	br i1 %t55, label %b1, label %b2
b1: ; preds = %b0
	%x56 = load i32, i32* %x7, align 4
	%x57 = srem i32 %x56, 7
	%t58 = icmp ne i32 %x57, 0
	%x59 = zext i1 %t58 to i32
	br label %b2
b2: ; preds = %b0, %b1
	%x60 = phi i32 [%x54, %b0], [%x59, %b1]
	; if %x60 then b3 else b12
	%t61 = icmp ne i32 %x60, 0
	br i1 %t61, label %b3, label %b12
b3: ; preds = %b2
	br label %b4
b4: ; preds = %b3, %b9, %b10
	%x62 = load i32, i32* %x5, align 4
	%t63 = icmp eq i32 %x62, 0
	%x64 = zext i1 %t63 to i32
	; if %x64 then b5 else b11
	%t65 = icmp ne i32 %x64, 0
	br i1 %t65, label %b5, label %b11
b5: ; preds = %b4
	%x66 = load i32, i32* %x5, align 4
	%x67 = load i32, i32* %x7, align 4
	%x68 = add i32 %x66, %x67
	; store 69
	store i32 %x68, i32* %x5, align 4
	%x70 = load i32, i32* %x5, align 4
	%x71 = srem i32 %x70, 7
	%t72 = icmp eq i32 %x71, 0
	%x73 = zext i1 %t72 to i32
	%t74 = icmp eq i32 %x73, 0
	%x75 = zext i1 %t74 to i32
	; if %x75 then b6 else b7
	%t76 = icmp ne i32 %x75, 0
	br i1 %t76, label %b6, label %b7
b6: ; preds = %b5
	%x77 = load i32, i32* %x5, align 4
	%t78 = icmp sgt i32 %x77, 100
	%x79 = zext i1 %t78 to i32
	br label %b7
b7: ; preds = %b5, %b6
	%x80 = phi i32 [%x73, %b5], [%x79, %b6]
	; if %x80 then b8 else b9
	%t81 = icmp ne i32 %x80, 0
	br i1 %t81, label %b8, label %b9
b8: ; preds = %b7
	br label %b11
b9: ; preds = %b7
	br label %b4
b10: ; preds = 
	br label %b4
b11: ; preds = %b4, %b8
	br label %b13
b12: ; preds = %b2
	br label %b13
b13: ; preds = %b11, %b12
	%x82 = load i32, i32* %x5, align 4
	%t83 = icmp sge i32 %x82, 30
	%x84 = zext i1 %t83 to i32
	; if %x84 then b14 else b15
	%t85 = icmp ne i32 %x84, 0
	br i1 %t85, label %b14, label %b15
b14: ; preds = %b13
	%x86 = load i32, i32* %x5, align 4
	%t87 = getelementptr inbounds [19 x i8], [19 x i8]* @.str24, i32 0, i32 0
	call void @putstr(i8* %t87)
	call void @putint(i32 %x86)
	%t88 = getelementptr inbounds [15 x i8], [15 x i8]* @.str25, i32 0, i32 0
	call void @putstr(i8* %t88)
	br label %b19
b15: ; preds = %b13
	%x89 = load i32, i32* %x5, align 4
	%t90 = icmp sle i32 %x89, 10
	%x91 = zext i1 %t90 to i32
	; if %x91 then b16 else b17
	%t92 = icmp ne i32 %x91, 0
	br i1 %t92, label %b16, label %b17
b16: ; preds = %b15
	%x93 = load i32, i32* %x5, align 4
	%t94 = getelementptr inbounds [19 x i8], [19 x i8]* @.str26, i32 0, i32 0
	call void @putstr(i8* %t94)
	call void @putint(i32 %x93)
	%t95 = getelementptr inbounds [15 x i8], [15 x i8]* @.str27, i32 0, i32 0
	call void @putstr(i8* %t95)
	br label %b18
b17: ; preds = %b15
	%x96 = load i32, i32* %x5, align 4
	%t97 = getelementptr inbounds [19 x i8], [19 x i8]* @.str28, i32 0, i32 0
	call void @putstr(i8* %t97)
	call void @putint(i32 %x96)
	%t98 = getelementptr inbounds [23 x i8], [23 x i8]* @.str29, i32 0, i32 0
	call void @putstr(i8* %t98)
	br label %b18
b18: ; preds = %b16, %b17
	br label %b19
b19: ; preds = %b14, %b18
	%x99 = load i32, i32* %x5, align 4
	%x100 = sub i32 0, %x99
	; store 101
	store i32 %x100, i32* %x5, align 4
	%x102 = load i32, i32* %x7, align 4
	%x103 = add i32 %x102, 0
	; store 104
	store i32 %x103, i32* %x7, align 4
	%t105 = getelementptr inbounds [50 x i8], [50 x i8]* @.str30, i32 0, i32 0
	call void @putstr(i8* %t105)
	ret i32 0
}

