// Collapsible dashboard blocks with persistence
(function() {
  if (!window.location.pathname.match(/^\/admin\/dashboard/)) return;
  document.addEventListener('DOMContentLoaded', function() {
    var blocks = document.querySelectorAll('.block-dashboard, .admin-panel');
    blocks.forEach(function(block, i) {
      var title = block.querySelector('.block-title');
      var content = block.querySelector('.block-content');
      if (!title || !content) return;
      // Use a unique key for each block (by index and title text)
      var blockId = 'dashboard-block-' + (title.textContent || i).replace(/\s+/g, '-').toLowerCase();
      // Add caret if not present
      if (!title.querySelector('.toggle-caret')) {
        var caret = document.createElement('span');
        caret.className = 'toggle-caret';
        caret.innerHTML = '&#9654;';
        title.appendChild(caret);
      }
      title.setAttribute('tabindex', '0');
      title.setAttribute('role', 'button');
      title.setAttribute('aria-controls', blockId + '-content');
      content.id = blockId + '-content';
      // Restore state
      var collapsed = localStorage.getItem(blockId) === 'collapsed';
      if (collapsed) {
        content.classList.add('hide');
        title.setAttribute('aria-expanded', 'false');
        title.querySelector('.toggle-caret').style.transform = '';
      } else {
        content.classList.remove('hide');
        title.setAttribute('aria-expanded', 'true');
        title.querySelector('.toggle-caret').style.transform = 'rotate(90deg)';
      }
      // Toggle handler
      function toggleBlock() {
        var isCollapsed = content.classList.toggle('hide');
        if (isCollapsed) {
          title.setAttribute('aria-expanded', 'false');
          title.querySelector('.toggle-caret').style.transform = '';
          localStorage.setItem(blockId, 'collapsed');
        } else {
          title.setAttribute('aria-expanded', 'true');
          title.querySelector('.toggle-caret').style.transform = 'rotate(90deg)';
          localStorage.setItem(blockId, 'expanded');
        }
      }
      title.addEventListener('click', toggleBlock);
      title.addEventListener('keydown', function(e) {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          toggleBlock();
        }
      });
    });
  });
})(); 