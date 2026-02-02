<?php
/**
 * @file
 * Dashboard block showing recent unpublished comments.
 */
class DashboardUnpublishedCommentsBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return t('Recent unpublished comments');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Dashboard Plus - Recent unpublished comments');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    return t('Shows the most recent unpublished comments.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return user_access('administer comments') || user_access('moderate comments');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!$this->checkAccess()) {
      return array();
    }
    $allowed = array(10, 25, 50, 100);
    $count = isset($_GET['unpub_comment_count']) && in_array((int) $_GET['unpub_comment_count'], $allowed) ? (int) $_GET['unpub_comment_count'] : 10;
    $form = '<form method="get" style="margin-bottom:8px;display:inline;white-space:nowrap;">'
      . '<label for="unpub-comment-count-select" style="display:inline;">' . t('Show') . '</label> '
      . '<select id="unpub-comment-count-select" name="unpub_comment_count" onchange="this.form.submit()" style="display:inline;">';
    foreach ($allowed as $option) {
      $selected = ($option == $count) ? ' selected' : '';
      $form .= '<option value="' . $option . '"' . $selected . '>' . $option . '</option>';
    }
    $form .= '</select> ' . t('comments') . '</form>';
    $query = db_select('comment', 'c')
      ->fields('c')
      ->condition('c.status', 0)
      ->orderBy('c.cid', 'DESC')
      ->range(0, $count);
    $result = $query->execute();
    $rows = array();
    foreach ($result as $row) {
      $author = check_plain($row->name ? $row->name : t('Anonymous'));
      $node = node_load($row->nid);
      $node_title = $node ? l(check_plain($node->title), 'node/' . $row->nid) : t('Unknown');
      $date = format_date($row->created, 'short');
      $moderate_link = l(t('Moderate'), 'comment/' . $row->cid . '/edit');
      $rows[] = array(
        'data' => array(
          $author,
          $node_title,
          $date,
          $moderate_link,
        ),
      );
    }
    $header = array(
      array('data' => t('Author')),
      array('data' => t('Node')),
      array('data' => t('Date')),
      array('data' => t('Moderate')),
    );
    $panel = array(
      '#theme' => 'dashboard_panel__unpublished_comments',
    );
    $panel['form'] = array(
      '#markup' => $form,
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No unpublished comments found.'),
    );
    $panel['link'] = array(
      '#theme' => 'link',
      '#path' => 'admin/content/comment/approval',
      '#text' => t('View all unpublished comments'),
    );
    return $panel;
  }
} 