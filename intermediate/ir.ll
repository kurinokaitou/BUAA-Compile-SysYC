@a = global i32 1
@c = constant i32 2
@arr = global [8 x i32] [i32 1, i32 2, i32 3, i32 4, i32 1, i32 2, i32 3, i32 4]
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() #0 {
_0: ; preds = 
	%t0 = alloca i32, align 4
	%x1 = getelementptr inbounds i32, i32* %t0, i32 0
	; store 2
	store i32 1, i32* %x1, align 4
	%t3 = getelementptr inbounds i32, i32* %x1, i32 0
	%x4 = load i32, i32* %t3, align 4
    %x5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %x4)
    ret i32 0
}


declare dso_local i32 @printf(i8*, ...) #1

