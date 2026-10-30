function openNewAdmin() {
  if (localStorage.getItem('isAdmin') === 'true') {
    document.getElementById('newAdminPanel').style.display = 'flex';
    document.getElementById('newAdminBubble').style.display = 'block';
    newRefreshLogs(); return;
  }
  var pass = prompt("Введите пароль:");
  if (pass === "1512") {
    localStorage.setItem('isAdmin', 'true');
    document.getElementById('newAdminBubble').style.display = 'block';
    document.getElementById('newAdminPanel').style.display = 'flex';
    newRefreshLogs();
  } else if (pass !== null) { alert("Неверный пароль!"); }
}
function newCloseAdmin() { document.getElementById('newAdminPanel').style.display = 'none'; }
function newClearChat() {
  if (confirm("Очистить чат?")) {
    fetch('/clear_chat', { method: 'POST' }).then(() => {
      alert("Чат очищен!");
      document.querySelectorAll('.messages').forEach(el => el.innerHTML = '');
    });
  }
}
function newToggleBubble() {
  var b = document.getElementById('newAdminBubble');
  b.style.display = (b.style.display === 'block') ? 'none' : 'block';
}
function newRefreshLogs() {
  var area = document.getElementById('newLogArea');
  if(!area) return;
  area.value = "Загрузка...";
  fetch('/get_logs').then(r => r.json()).then(d => {
    area.value = d.join('\n'); area.scrollTop = area.scrollHeight;
  }).catch(e => area.value = "Ошибка: " + e);
}
document.addEventListener('DOMContentLoaded', () => {
  if (localStorage.getItem('isAdmin') === 'true') {
    var b = document.getElementById('newAdminBubble'); if(b) b.style.display = 'block';
  }
});
