 const lognumbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49];
  
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
 	let prev_lognumber = lognumbers[lognumbers.length-1]; 
 	for (let i = 1; i < lognumbers.length; i++) { 
 		if (lognumbers[i] === lognumber) { 
 			prev_lognumber = lognumbers[i-1]; 
 			break; 
 		} 
 	} 
 	const page = '../' + prev_lognumber + '/index.html'; 
 	window.location.href = page; 
 } 
