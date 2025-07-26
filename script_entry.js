 const lognumbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116];
  
 function Next() { 
 	const text_lognumber = document.getElementById('lognumber').textContent; 
 	const lognumber = Number(text_lognumber); 
 	let next_lognumber = lognumbers[0]; 
 	for (let i = 0; i < lognumbers.length - 1; i++) { 
 		if (lognumbers[i] === lognumber) { 
 			next_lognumber = lognumbers[i+1]; 
 			break; 
 		} 
 	} 
 	const page = '../' + next_lognumber + '/index.html'; 
 	window.location.href = page; 
 } 
  
 function Prev() { 
 	const text_lognumber = document.getElementById('lognumber').textContent; 
 	const lognumber = Number(text_lognumber); 
 	let prev_lognumber = lognumbers[0]; 
 	for (let i = 1; i < lognumbers.length; i++) { 
 		if (lognumbers[i] === lognumber) { 
 			prev_lognumber = lognumbers[i-1]; 
 			break; 
 		} 
 	} 
 	const page = '../' + prev_lognumber + '/index.html'; 
 	window.location.href = page; 
 } 
