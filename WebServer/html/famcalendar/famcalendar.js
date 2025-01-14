function generateCalendar() {
  // Get today's date
  const today = new Date();

  // Set end date to one year from today
  const endDate = new Date(today.getFullYear() + 1, today.getMonth(), today.getDate());

  // Create the table element
  const table = document.createElement('table');

  // Create the header row for days
  const headerRow = document.createElement('tr');
  for (let day = 0; day < 7; day++) {
    const dayName = new Intl.DateTimeFormat('en-US', { weekday: 'short' }).format(new Date(today.setDate(today.getDate() + day)));
    const headerCell = document.createElement('th');
    headerCell.textContent = dayName;
    headerRow.appendChild(headerCell);
  }
  table.appendChild(headerRow);

  // Loop through each week until end date is reached
  let currentDate = today;
  while (currentDate <= endDate) {
    const row = document.createElement('tr');

    // Add cell for date (optional)
    const dateCell = document.createElement('td');
    dateCell.textContent = currentDate.toLocaleDateString('en-US', { weekday: 'long', year: 'numeric', month: 'short', day: 'numeric' });
    row.appendChild(dateCell);

    // Add cells for each hour
    for (let hour = 0; hour < 24; hour++) {
      const cell = document.createElement('td');
      cell.textContent = hour.toString().padStart(2, '0') + ":00";
      row.appendChild(cell);
    }

    table.appendChild(row);

    // Move to next week
    currentDate.setDate(currentDate.getDate() + 7);
  }

  // Add the table to the document body (replace with your desired container)
  document.body.appendChild(table);

  // Add scroll functionality (optional)
  table.style.width = '100%'; // Set table width to full size
  table.style.overflowX = 'scroll'; // Enable horizontal scrolling

  console.log("table created");
}
console.log("table creation started");
generateCalendar();