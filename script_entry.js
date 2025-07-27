 const lognumbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13];
  
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
