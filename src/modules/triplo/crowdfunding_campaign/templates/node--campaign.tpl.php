<?php
/**
 * @file
 * Default theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: An array of node items. Use render($content) to print them all,
 *   or print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $user_picture: The node author's picture from user-picture.tpl.php.
 * - $date: Formatted creation date. Preprocess functions can reformat it by
 *   calling format_date() with the desired parameters on the $created variable.
 * - $name: Themed username of node author output from theme_username().
 * - $node_url: Direct URL of the current node.
 * - $display_submitted: Whether submission information should be displayed.
 * - $submitted: Submission information created from $name and $date during
 *   template_preprocess_node().
 * - $classes: Array of classes that can be used to style contextually through
 *   CSS. The default values can be one or more of the following:
 *   - node: The current template type; for example, "theme hook".
 *   - node-[type]: The current node type. For example, if the node is a
 *     "Post" it would result in "node-post". Note that the machine
 *     name will often be in a short form of the human readable label.
 *   - view-mode-[view_mode]: The View Mode of the node e.g. teaser or full.
 *   - preview: Nodes in preview mode.
 *   The following are controlled through the node publishing options.
 *   - promoted: Nodes that are promoted.
 *   - sticky: Nodes ordered above other non-sticky nodes in teaser
 *     listings.
 *   - unpublished: Unpublished nodes visible only to administrators.
 * - $attributes: Array of additional HTML attributes that should be added to
 *   the wrapper element. Flatten with backdrop_attributes().
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * Other variables:
 * - $node: Full node entity. Contains data that may not be safe.
 * - $type: Node type; for example, post, page, blog, etc.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 * - $comment_count: Number of comments attached to the node.
 * - $comments: The comment-related elements for the node. Contains two values
 *   that are both arrays that need to be rendered through a render() call:
 *   - $comments['comments']: Comments for this node.
 *   - $comments['comment_form']: Form for adding a new comment.
 * - $comment_display_mode: Whether comments are threaded or not, one of the
 *   following PHP constants:
 *   - COMMENT_MODE_FLAT
 *   - COMMENT_MODE_THREADED
 *
 * Node status variables:
 * - $view_mode: Display mode, e.g. 'full', or 'teaser'.
 * - $teaser: Flag for the teaser state (shortcut for $view_mode == 'teaser').
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * Field variables: for each field instance attached to the node a corresponding
 * variable is defined; for example, $node->body becomes $body. When needing to
 * access a field's raw values, developers/themers are strongly encouraged to
 * use these variables. Otherwise they will have to explicitly specify the
 * desired field language; for example, $node->body['en'], thus overriding any
 * language negotiation rule that was previously applied.
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 *
 * @ingroup themeable
 */
?>
<article id="node-<?php print $node->nid; ?>" class="<?php print implode(' ', $classes); ?> clearfix"<?php print backdrop_attributes($attributes); ?>>

  <div class="hero_image_wrapper">
    <?php print render($content['field_hero_image']); ?>
    <div class="hero_page_title">
      <div class="container"></div>
    </div>
  </div>

  <?php print render($title_prefix); ?>
  <?php if (!$page && !empty($title)): ?>
    <h2><a href="<?php print $node_url; ?>" rel="bookmark"><?php print $title; ?></a></h2>
  <?php endif; ?>
  <?php print render($title_suffix); ?>

  <?php if ($display_submitted): ?>
    <footer>
      <?php print $user_picture; ?>
      <p class="submitted"><?php print $submitted; ?></p>
    </footer>
  <?php endif; ?>

  <div class="content crowdfunding clearfix"<?php print backdrop_attributes($content_attributes); ?>>
    <?php
      // We hide the links now so that we can render them later.
      hide($content['links']);
    ?>
      <section class="leftside">
        <p class="campaign-thumbnail"><img src="<?php print render($content['field_thumbnail_link']['#items'][0]['url']); ?>"></p>
        <?php print render($content['body'][0]['#markup']); ?>
        <div class="campaign-teaser"> <?php print render($content['field_teaser'][0]['#markup']); ?> </div>
        <div class="campaign-caveat"> <?php print render($content['field_caveat_warning'][0]['#markup']); ?> </div>
        <div class="campaign-copyright"> <?php print render($content['field_caveat_copyright'][0]['#markup']); ?></div>
      </section>
      <?php 
        $visibility = $node->field_visibility['und'][0]['value'];
        session_start();
      ?>
      <section class="rightside">
        <?php if ($visibility == 'open' || (isset($_SESSION['sppx']['access_token']) && !empty($_SESSION['sppx']['access_token']))) :?>
          <div class="row">
            <div class="rightside_top">
              <div class="item">
                <div class="label"> ID:</div>
                <div> $<?php print render($content['field_sppx_id'][0]['#markup']); ?> </div>
              </div>

              <div class="item">
                <div class="label"> Type:</div>
                <div> $<?php print render($content['field_type'][0]['#markup']); ?> </div>
              </div>
            </div>
          </div>
          <div class="rightside_inner">

            <div class="block_item">
              <div class="block_item_title campaign_raise">Raise</div>
              <div class="block_item_inner three_col">
                <div class="item">
                  <div class="label campaign-minimum-raise"> Minimum Raise:</div>
                  <div> $<?php print render($content['field_raise_minimum'][0]['#markup']); ?> </div>
                </div>
                <div class="item">
                  <div class="label campaign-maximum-raise"> Maximum Raise:</div>
                  <div> $<?php print render($content['field_raise_maximum'][0]['#markup']); ?> </div>
                </div>
                <div class="item">
                  <div class="label campaign-target-raise"> Target Raise:</div>
                  <div> $<?php print render($content['field_raise_target'][0]['#markup']); ?> </div>
                </div>
              </div>
            </div>

            <div class="block_item">
              <div class="block_item_title">Raised so far</div>
              <div class="block_item_inner three_col">
                <div class="item">
                  <div class="label campaign-raised"> Raised so far:</div>
                  <div> $<?php print render($content['field_raised'][0]['#markup']); ?> </div>
                </div>
                <div class="item">
                  <div class="label campaign-lotsize"> Lotsize:</div>
                  <div> <?php print render($content['field_lotsize'][0]['#markup']); ?> </div>
                </div>
                <div class="item">
                  <div class="label campaign-status"> Status:</div>
                  <div> <?php print render($content['field_status'][0]['#markup']); ?> </div>
                </div>
              </div>
            </div>

            <div class="block_item">
              <div class="block_item_title campaign-date">Date</div>
              <div class="block_item_inner two_col">
                <div class="item campaign-start-date">
                  <div class="label"> Start Date:</div>
                  <div><?php print render($content['field_start_date'][0]['#markup']); ?> </div>
                </div>
                <div class="item campaign-end-date">
                  <div class="label"> End Date:</div>
                  <div><?php print render($content['field_end_date'][0]['#markup']); ?> </div>
                </div>
              </div>
            </div>
            <div class="forum">
              <a href="<?php print render($content['field_forum_link']['#items'][0]['display_url']); ?>"> Forum</a>
            </div>
          </div>
        <?php else: ?>
          <div class="not-open">To see the details of this campaign, we need to verify your Silicon Prairie account here</div>
          <?php  print backdrop_render($sppx_login_form); ?>
          <?php endif;?>
      </section>
  </div>
  
  <?php print render($content['links']); ?>

</article>


